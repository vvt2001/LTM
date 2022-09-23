#include "stdio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "iostream"
#include "string"
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#define BUFF_SIZE 2048
#define USER "USER "
#define POST "POST "
#define QUIT "QUIT "
#define ENDING_DELIMITER "\r\n"
#define SUCCESS_LOGIN "10"
#define FAILED_LOGIN "11"
#define LOCKED_LOGIN "12"
#define SUCCESS_POST "20"
#define SUCCESS_LOGOUT "30"


int main(int argc, char* argv[])
{
	char portChar[100], serverIp[100];
	strcpy_s(serverIp, argv[1]);
	strcpy_s(portChar, argv[2]);
	int serverPort;
	sscanf_s(portChar, "%d", &serverPort);

	//step1: initiate winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//step2: construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket", WSAGetLastError());
	}

	//optional set timeout for receiving 
	int tv = 10000;//timeout interval:10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));


	//step3: specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);

	//step4: communicate with server
	if (connect(client, (sockaddr *)&serverAddr,
		sizeof(serverAddr))) {
		printf("Error! Cannot connect server.\n");
		return 0;
	}
	printf("Connected to server\n");

	//step5: communicate with server
	char buff[BUFF_SIZE];
	int ret, messageLen;
	bool checkLogin = false;

	//Send message
	while (1) {
		string message;
		//Login
		while (checkLogin == false) {
			printf("Login(enter nothing to exit): ");
			gets_s(buff, BUFF_SIZE);
			message = buff;
			messageLen = strlen(buff);

			//if client wants to quit
			if (messageLen == 0) {
				//adding identifier & delimiter
				message = QUIT + message + ENDING_DELIMITER;
				strcpy_s(buff, &message[0]);

				ret = send(client, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d\n", WSAGetLastError());

				memset(buff, 0, BUFF_SIZE);
				ret = recv(client, buff, BUFF_SIZE, 0);
				if (ret == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT)
						printf("Time-out!\n");
					else
						printf("Error %d: Cannot receive data\n", WSAGetLastError());
				}
				else {
					printf("Quitted.\n\n");
					break;
				}
			}
			else {
				//adding identifier & delimiter
				message = USER + message + ENDING_DELIMITER;
				strcpy_s(buff, &message[0]);

				ret = send(client, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d\n", WSAGetLastError());

				//Receive response
				memset(buff, 0, BUFF_SIZE);
				ret = recv(client, buff, BUFF_SIZE, 0);
				if (ret == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT)
						printf("Time-out!\n");
					else printf("Error %d: Cannot receive data\n", WSAGetLastError());
				}
				else if (strcmp(buff, "10") == 0) { //if logged in, break while
					printf("Login successfully\n\n");
					checkLogin = true;
					break;
				}
				else if (strcmp(buff, "11") == 0) printf("Username incorrect! Try again!\n\n");
				else if (strcmp(buff, "12") == 0) printf("Username locked! Try another one!\n\n");
			}//end while login
		}

		//if client didn't enter username, exit
		if (messageLen == 0) break;

		//Upload message
		memset(buff, 0, BUFF_SIZE);
		while (checkLogin == true) {
			printf("Upload(enter nothing to log out): ");
			gets_s(buff, BUFF_SIZE);
			message = buff;
			messageLen = strlen(buff);

			//if client wants to log out
			if (messageLen == 0) {
				//adding identifier & delimiter
				message = QUIT + message + ENDING_DELIMITER;
				strcpy_s(buff, &message[0]);

				ret = send(client, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d\n", WSAGetLastError());

				//receive response
				memset(buff, 0, BUFF_SIZE);
				ret = recv(client, buff, BUFF_SIZE, 0);
				if (ret == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT)
						printf("Time-out!\n");
					else
						printf("Error %d: Cannot receive data\n", WSAGetLastError());
				}
				else {
					printf("Logged out.\n\n");
					checkLogin = false;
					break;
				}

			}

			else {
				//adding delimiter
				message = POST + message + ENDING_DELIMITER;
				strcpy_s(buff, &message[0]);

				ret = send(client, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d\n", WSAGetLastError());

				//receive response
				memset(buff, 0, BUFF_SIZE);
				ret = recv(client, buff, BUFF_SIZE, 0);
				if (ret == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT)
						printf("Time-out!\n");
					else printf("Error %d: Cannot receive data\n", WSAGetLastError());
				}
				else if (strcmp(buff, "20") == 0) {
					printf("Upload successfully\n\n");
				}
			}
		}//end while upload
	}
	//step 6: close socket
	closesocket(client);
	//step7: terminate winsock
	WSACleanup();
	return 0;
}
