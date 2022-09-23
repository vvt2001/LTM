// WSAEventSelectServer.cpp : Defines the entry point for the console application.
//

#include "winsock2.h"
#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
#include "string.h"
#include "ws2tcpip.h"
#include "fstream"
#include "iostream"
#include "string"
#include "chrono"
#include "ctime"
using namespace std;

#define WM_SOCKET WM_USER + 1
#define MAX_CLIENT 1024
#define BUFF_SIZE 2048
#define SERVER_ADDR "127.0.0.1"
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
#define SYNTAX_ERROR "40"

struct clientIf {
	SOCKET socket;
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;
	bool login;
};
clientIf connList[MAX_CLIENT];

int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);
void processData(string &, clientIf &);

int main(int argc, char* argv[])
{
	DWORD		nEvents = 0;
	DWORD		index;
	WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS sockEvent;

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

	//Step 2: Construct LISTEN socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	connList[0].socket = listenSock;
	events[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(connList[0].socket, events[0], FD_ACCEPT | FD_CLOSE);


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

	char rcvBuff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr), clientPort;
	int ret, i;
	string messageQueue;
	string rcvMessage;

	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		connList[i].socket = 0;
	}
	while (1) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(connList[index].socket, events[index], &sockEvent);

		//reset event
		WSAResetEvent(events[index]);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
			}

			if ((connSock = accept(connList[index].socket, (sockaddr *)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
			}

			//Add new socket into socks array
			int i;
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);

			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				printf("\nToo many clients.");
				closesocket(connSock);
			}
			else {
				connList[nEvents].socket = connSock;
				events[nEvents] = WSACreateEvent();
				strcpy_s(connList[nEvents].clientIP, clientIP);
				connList[nEvents].clientPort = clientPort;
				connList[nEvents].login = 0;
				WSAEventSelect(connList[nEvents].socket, events[nEvents], FD_READ | FD_CLOSE);
				nEvents++;
			}

		}

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
			}

			ret = recv(connList[index].socket, rcvBuff, BUFF_SIZE, 0);

			//Release socket and event if an error occurs
			if (ret <= 0) {
				closesocket(connList[index].socket);
				connList[index].socket = 0;
				WSACloseEvent(events[index]);
				nEvents--;
			}
			else {
				//process & send back to client
				rcvBuff[ret] = 0;

				//adding new received buff to message queue to process
				rcvMessage = rcvBuff;
				memset(rcvBuff, 0, BUFF_SIZE);
				messageQueue = messageQueue + rcvMessage;

				processData(messageQueue, connList[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
			}
			//Release socket and event
			closesocket(connList[index].socket);
			WSACloseEvent(events[index]);
			connList[index] = connList[nEvents - 1];
			events[index] = events[nEvents - 1];
			connList[nEvents - 1].socket = 0;
			connList[nEvents - 1].login = 0;
			events[nEvents - 1] = 0;
			nEvents--;
		}
	}
	return 0;
}

//function to find time 
string findTime() {
	time_t now = time(NULL);
	char str[26] = {};
	ctime_s(str, 26, &now);
	string timeLog = str;
	timeLog.pop_back();
	return timeLog;
}

//function to write to logs
void writeLog(clientIf &client, string &data, string currentStatus) {
	string time = findTime();
	//open logs file
	ofstream fileLogs;
	fileLogs.open("log_20194374.txt", ios::out | ios::app);

	//save logs to file "log_20194374.txt"
	fileLogs << client.clientIP << ":" << client.clientPort << " [" << time << "] $ " << data << " $ " << currentStatus << endl;

	//close logs file
	fileLogs.close();
}

//function to process when login
void userLogin(clientIf &client, string &data) {
	char buff[BUFF_SIZE];
	int ret;
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
						strcpy_s(buff, LOCKED_LOGIN);
						break;
					}
					if (status == '1') {
						strcpy_s(buff, SUCCESS_LOGIN);
						client.login = true;
						break;
					}
				}
				else strcpy_s(buff, FAILED_LOGIN);
			}
		}
		string currentStatus;
		if (strcmp(buff, SUCCESS_LOGIN) == 0) currentStatus = SUCCESS_LOGIN;
		if (strcmp(buff, LOCKED_LOGIN) == 0) currentStatus = LOCKED_LOGIN;
		if (strcmp(buff, FAILED_LOGIN) == 0) currentStatus = FAILED_LOGIN;

		data = "USER " + data;

		//save logs to file "log_20194374.txt"
		writeLog(client, data, currentStatus);

		//response to client
		ret = send(client.socket, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
	}
	else {
		writeLog(client, data, FAILED_LOGIN);

		strcpy_s(buff, FAILED_LOGIN);
		ret = send(client.socket, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
	}
}

//function to process when posting
void post(clientIf &client, string &data) {
	char buff[BUFF_SIZE];
	int ret;
	if (client.login == true) {
		//save logs to file "log_20194374.txt"
		writeLog(client, data, SUCCESS_POST);

		//response to client
		memset(buff, 0, BUFF_SIZE);
		strcpy_s(buff, SUCCESS_POST);
		ret = send(client.socket, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
	}
	else {
		//save logs to file "log_20194374.txt"
		writeLog(client, data, FAILED_POST);

		//response to client
		memset(buff, 0, BUFF_SIZE);
		strcpy_s(buff, FAILED_POST);
		ret = send(client.socket, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
	}
}

//function to process when client quit/log out
void quit(clientIf &client, string &data) {
	char buff[BUFF_SIZE];
	int ret;
	//save logs to file "log_20194374.txt"
	writeLog(client, data, SUCCESS_LOGOUT);

	//response to client
	memset(buff, 0, BUFF_SIZE);
	strcpy_s(buff, SUCCESS_LOGOUT);
	ret = send(client.socket, buff, strlen(buff), 0);
	if (ret == SOCKET_ERROR)
		printf("Error %d: Cannot send data.\n", WSAGetLastError());
	client.login = false;
}

//function to handle syntax error
void hSyntaxError(clientIf &client, string &data) {
	char buff[BUFF_SIZE];
	int ret;
	//save logs to file "log_20194374.txt"
	writeLog(client, data, SYNTAX_ERROR);

	//response to client
	memset(buff, 0, BUFF_SIZE);
	strcpy_s(buff, SYNTAX_ERROR);
	ret = send(client.socket, buff, strlen(buff), 0);
	if (ret == SOCKET_ERROR)
		printf("Error %d: Cannot send data.\n", WSAGetLastError());
}

//function to process message queue & handle stream
void processData(string &messageQueue, clientIf &client) {
	int ret;
	string time;

	//find time
	time = findTime();

	//find delimiter to process
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
			userLogin(client, data);
		}
		//if it was a normal message
		if (postPos == 0) {
			post(client, data);
		}
		//if client wants to quit
		if (quitPos == 0) {
			quit(client, data);
		}
		//if syntax error
		if (userPos != 0 && postPos != 0 && quitPos != 0) {
			hSyntaxError(client, data);
		}

		//clear processed data 
		//	Sleep(10);
		messageQueue = messageQueue.substr(end_deliPos + 2, messageQueue.length() - 1);

		//if there is stil processable data -> continue
		if (messageQueue != "") end_deliPos = messageQueue.find(ENDING_DELIMITER);
		else break;
	}
}