#include "stdio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "iostream"
#include "string"
using namespace std;
#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
	char portChar[100], ip[100];
	strcpy_s(ip, argv[1]);
	strcpy_s(portChar, argv[2]);
	int port;
	sscanf_s(portChar, "%d", &port);

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
	int tv = 100000;//timeout interval:100000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));


	//step3: specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serverAddr.sin_addr);

	//step4: communicate with server
	if (connect(client, (sockaddr *)&serverAddr,
		sizeof(serverAddr))) {
		printf("Error! Cannot connect server.");
		return 0;
	}
	printf("Connected to server\n");

	 //step5: communicate with server
	char buff[BUFF_SIZE];
	int ret, messageLen;
	//Send message
	while (1) {
		//Send message
		string message;
		printf("Send to server: ");
		getline(cin, message);
		messageLen = message.length();
		if (messageLen == 0) break;

		message = message + ENDING_DELIMITER; //adding delimiter to message
		strcpy_s(buff, &message[0]);

		ret = send(client, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d\n", WSAGetLastError());

		//Receive message
		ret = recv(client, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!\n");
			else 
				printf("Error %d: Cannot receive data\n", WSAGetLastError());
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("%s\n", buff);
		}
	}
	//step 6: close socket
	closesocket(client);
	//step7: terminate winsock
	WSACleanup();
	return 0;
}


//Test1
/*
#include "stdio.h"
#include "conio.h"
#include "string.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "process.h"
#define BUFF_SIZE 1024
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define ENDING_DELIMITER "\r\n"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported \n");

	char buff[BUFF_SIZE];
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &(serverAddr.sin_addr.s_addr));

	SOCKET connSock;
	connSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(connSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("\nError: %d", WSAGetLastError());
		closesocket(connSock);
		return 0;
	}

	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];
	int ret;

	strcpy_s(sBuff, "123");
	strcat_s(sBuff, ENDING_DELIMITER);
	strcat_s(sBuff, "a@b");
	strcat_s(sBuff, ENDING_DELIMITER);
	strcat_s(sBuff, "456");
	strcat_s(sBuff, ENDING_DELIMITER);

	ret = send(connSock, sBuff, strlen(sBuff), 0); Sleep(100);
	ret = recv(connSock, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	ret = recv(connSock, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	ret = recv(connSock, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	closesocket(connSock);
	WSACleanup();

	return 0;
}
*/


