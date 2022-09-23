// SelectTCPServer.cpp : Defines the entry point for the console application.
//
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
#define SUCCESS_LOGIN "10"
#define FAILED_LOGIN "11"
#define LOCKED_LOGIN "12"
#define SUCCESS_POST "20"
#define FAILED_POST "21"
#define SUCCESS_LOGOUT "30"

struct clientIf {
	SOCKET socket;
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;
	bool login;
};
clientIf connList[100];

void processData(string &, clientIf &);
int Receive(SOCKET, char *, int, int);

int main(int argc, char* argv[])
{
	char portChar[100];
	strcpy_s(portChar, argv[1]);
	int serverPort;
	sscanf_s(portChar, "%d", &serverPort);

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}

	printf("Server started!\n");


	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int ret, nEvents, clientAddrLen, clientPort;
	char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	string message;
	string messageQueue;

	for (int i = 0; i < FD_SETSIZE; i++)
		connList[i].socket = 0;	// 0 indicates available entry

	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Step 5: Communicate with clients
	
	while (1) {
		
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				int i;
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
				clientPort = ntohs(clientAddr.sin_port);
				printf("You got a connection from %s:%d\n", clientIP, clientPort); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (connList[i].socket == 0) {
						connList[i].socket = connSock;
						strcpy(connList[i].clientIP,clientIP);
						connList[i].clientPort = clientPort;
						connList[i].login = false;
						FD_SET(connList[i].socket, &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}
		
		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (connList[i].socket == 0)
				continue;

			if (FD_ISSET(connList[i].socket, &readfds)) {
				ret = Receive(connList[i].socket, rcvBuff, BUFF_SIZE, 0);
				if (ret <= 0) {
					FD_CLR(connList[i].socket, &initfds);
					closesocket(connList[i].socket);
					connList[i].socket = 0;
				}
				else if (ret > 0) {
					rcvBuff[ret] = 0;

					message = rcvBuff;
					memset(rcvBuff, 0, BUFF_SIZE);
					messageQueue = messageQueue + message;

					processData(messageQueue, connList[i]);
				}
			}

			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	closesocket(listenSock);
	WSACleanup();
	return 0;
}

//process function after receive message
void processData(string &messageQueue, clientIf &client) {
	int ret;
//	string message;
//	string messageQueue;
	char buff[BUFF_SIZE];

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
			if (client.login == false) {
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
								strcpy(buff, LOCKED_LOGIN);
								break;
							}
							if (status == '1') {
								strcpy(buff, SUCCESS_LOGIN);
								client.login = true;
								break;
							}
						}
						else strcpy(buff, FAILED_LOGIN);
					}
				}

				string currentStatus;
				if (strcmp(buff, SUCCESS_LOGIN) == 0) currentStatus = SUCCESS_LOGIN;
				if (strcmp(buff, LOCKED_LOGIN) == 0) currentStatus = LOCKED_LOGIN;
				if (strcmp(buff, FAILED_LOGIN) == 0) currentStatus = FAILED_LOGIN;

				//save logs to file "log_20194374.txt"
				fileLogs << client.clientIP << ":" << client.clientPort << " [" << timeLog << "] $ " << "USER " << data << " $ " << currentStatus << endl;

				ret = send(client.socket, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.\n", WSAGetLastError());
			}
			else {
				fileLogs << client.clientIP << ":" << client.clientPort << " [" << timeLog << "] $ " << "USER " << data << " $ " << FAILED_LOGIN << endl;
				strcpy(buff, FAILED_LOGIN);
				ret = send(client.socket, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.\n", WSAGetLastError());
			}
		}//end check login


		 //if it was a normal message
		if (postPos == 0) {
			if (client.login == true) {
				//save logs to file "log_20194374.txt"
				fileLogs << client.clientIP << ":" << client.clientPort << " [" << timeLog << "] $ " << data << " $ " << SUCCESS_POST << endl;

				//response to client
				memset(buff, 0, BUFF_SIZE);
				strcpy(buff, SUCCESS_POST);
				ret = send(client.socket, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.\n", WSAGetLastError());
			}
			else {
				//save logs to file "log_20194374.txt"
				fileLogs << client.clientIP << ":" << client.clientPort << " [" << timeLog << "] $ " << data << " $ " << FAILED_POST << endl;

				//response to client
				memset(buff, 0, BUFF_SIZE);
				strcpy(buff, FAILED_POST);
				ret = send(client.socket, buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR)
					printf("Error %d: Cannot send data.\n", WSAGetLastError());
			}
		}
		//end process normal message

		 //if client wants to quit
		if (quitPos == 0) {
			//save logs to file "log_20194374.txt"
			fileLogs << client.clientIP << ":" << client.clientPort << " [" << timeLog << "] $ " << "QUIT" << " $ " << SUCCESS_LOGOUT << endl;

			//response to client
			memset(buff, 0, BUFF_SIZE);
			strcpy(buff, SUCCESS_LOGOUT);
			ret = send(client.socket, buff, strlen(buff), 0);
			if (ret == SOCKET_ERROR)
				printf("Error %d: Cannot send data.\n", WSAGetLastError());
			client.login = false;
		}

		//clear processed data 
	//	Sleep(10);
		messageQueue = messageQueue.substr(end_deliPos + 2, messageQueue.length() - 1);

		//if there is stil processable data -> continue
		if (messageQueue != "") end_deliPos = messageQueue.find(ENDING_DELIMITER);
		else break;
	}
	memset(buff, 0, BUFF_SIZE);
}

/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %", WSAGetLastError());

	return n;
}
