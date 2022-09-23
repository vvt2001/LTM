#include "stdio.h"
#include "conio.h"
#include "string.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "process.h"
#include "fstream"
#include "iostream"
#include "string"
#include "chrono"
#include "ctime"
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define USER "USER "
#define POST "POST "
#define QUIT "QUIT "
#define ENDING_DELIMITER "\r\n"
#define bruh 3;

struct clientIf {
	SOCKET socket;
	sockaddr_in clientAddrThread;
};

/* handleThread - Thread to receive the message from client and process*/
unsigned __stdcall handleThread(void* param) {
	clientIf* pr = ((clientIf*)param);
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;
	inet_ntop(AF_INET, &(pr->clientAddrThread).sin_addr, clientIP, sizeof(clientIP));
	clientPort = ntohs((pr->clientAddrThread).sin_port);

	SOCKET connSock = pr->socket;
	char buff[BUFF_SIZE];
	int ret;
	string message;
	string messageQueue;

	while (1) {
		ret = recv(connSock, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
			break;
		}

		//if client forced disconnects
		else if (ret == 0) {
			printf("%s:%d disconnected.\n", clientIP, clientPort);
			break;
		}

		//if received message
		else if (strlen(buff) > 0) {
			//find time
			time_t now = time(NULL);
			char str[26] = {};
			ctime_s(str, 26, &now);
			string timeLog = str;
			timeLog.pop_back();

			//open logs file
			ofstream fileLogs;
			fileLogs.open("log_20194374.txt", ios::out | ios::app);

			//process
			buff[ret] = 0;
			message = buff;
			memset(buff, 0, BUFF_SIZE);
			messageQueue = messageQueue + message;
			int end_deliPos = messageQueue.find(ENDING_DELIMITER);

			//if found ending delimiter -> process
			while (end_deliPos >= 0) {

				//get data from message queue
				string data = messageQueue.substr(0, end_deliPos);

				int userPos = data.find(USER);
				int postPos = data.find(POST);
				int quitPos = data.find(QUIT);

				//if it was a login request
				if (userPos == 0) {
					data = data.substr(5, data.length() - 5);//remove user identifier
					ifstream fileAcc;
					fileAcc.open("account.txt");
					if (fileAcc.is_open()) {
						string line;
						while (getline(fileAcc, line)) {
							char status = line.back();
							line.pop_back();
							line.pop_back();
							if (data == line) {
								if (status == '0') {
									strcpy_s(buff, "0");
									break;
								}
								if (status == '1') {
									strcpy_s(buff, "1");
									break;
								}
							}
							else strcpy_s(buff, "-1");
						}
					}

					string currentStatus;
					if (strcmp(buff, "1") == 0) currentStatus = "+logged_in";
					if (strcmp(buff, "0") == 0) currentStatus = "-locked";
					if (strcmp(buff, "-1") == 0) currentStatus = "-wrong";

					//save logs to file "log_20194374.txt"
					fileLogs << clientIP << ":" << clientPort << " [" << timeLog << "] $ " << "USER " << data << " $ " << currentStatus << endl;

					ret = send(connSock, buff, strlen(buff), 0);
					if (ret == SOCKET_ERROR)
						printf("Error %d: Cannot send data.\n", WSAGetLastError());
				}//end check login


				 //if it was a normal message
				if (postPos == 0) {
					//save logs to file "log_20194374.txt"
					fileLogs << clientIP << ":" << clientPort << " [" << timeLog << "] $ " << data << " $ " << "+posted" << endl;

					//response to client
					memset(buff, 0, BUFF_SIZE);
					strcpy_s(buff, "success");
					ret = send(connSock, buff, strlen(buff), 0);
					if (ret == SOCKET_ERROR)
						printf("Error %d: Cannot send data.\n", WSAGetLastError());
				}//end process normal message

				 //if client wants to quit
				if (quitPos == 0) {
					//save logs to file "log_20194374.txt"
					fileLogs << clientIP << ":" << clientPort << " [" << timeLog << "] $ " << "QUIT" << " $ " << "+quitted" << endl;

					//response to client
					memset(buff, 0, BUFF_SIZE);
					strcpy_s(buff, "quitted");
					ret = send(connSock, buff, strlen(buff), 0);
					if (ret == SOCKET_ERROR)
						printf("Error %d: Cannot send data.\n", WSAGetLastError());
				}

				//clear processed data 
				Sleep(70);
				messageQueue = messageQueue.substr(end_deliPos + 2, messageQueue.length() - 1);

				//if there is stil processable data -> continue
				if (messageQueue != "") end_deliPos = messageQueue.find(ENDING_DELIMITER);
				else break;
			}
		}
	}
	closesocket(connSock);
	return 0;
}

int main(int argc, char* argv[])
{
	char portChar[100];
	strcpy_s(portChar, argv[1]);
	int serverPort;
	sscanf_s(portChar, "%d", &serverPort);

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Step 2: Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	//Step 5: Communicate with client
	SOCKET connSock;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr), clientPort;
	while (1) {
		connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		clientIf pr;
		pr.socket = connSock;
		pr.clientAddrThread = clientAddr;

		if (connSock == SOCKET_ERROR)
			printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
		else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Accept incoming connection from %s:%d\n", clientIP, clientPort);
			_beginthreadex(0, 0, handleThread, (void *)&pr, 0, 0); //start thread
		}
	}

	closesocket(listenSock);

	WSACleanup();

	return 0;
}