// WSAAsyncSelectServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AsyncSelectServer.h"
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
#define SERVER_PORT 6000
#define MAX_CLIENT 1024
#define BUFF_SIZE 2048
#define FD_SETSIZE 1024
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

void processData(string &, clientIf &);

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	windowProc(HWND, UINT, WPARAM, LPARAM);

SOCKET client[MAX_CLIENT];
SOCKET listenSock;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	MSG msg;
	HWND serverWindow;

	//Registering the Window Class
	MyRegisterClass(hInstance);

	//Create the window
	if ((serverWindow = InitInstance(hInstance, nCmdShow)) == NULL)
		return FALSE;

	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		MessageBox(serverWindow, L"Winsock 2.2 is not supported.", L"Error!", MB_OK);
		return 0;
	}

	//Construct socket	
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//requests Windows message-based notification of network events for listenSock
	WSAAsyncSelect(listenSock, serverWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		MessageBox(serverWindow, L"Cannot associate a local address with server socket.", L"Error!", MB_OK);
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		MessageBox(serverWindow, L"Cannot place server socket in state LISTEN.", L"Error!", MB_OK);
		return 0;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASYNCSELECTSERVER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	int i;
	for (i = 0; i <MAX_CLIENT; i++)
		connList[i].socket = 0;
	hWnd = CreateWindow(L"WindowClass", L"WSAAsyncSelect TCP Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_SOCKET	- process the events on the sockets
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SOCKET connSock;
	sockaddr_in clientAddr;
	int ret, clientAddrLen = sizeof(clientAddr), i, clientPort;
	char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	string messageQueue;
	string rcvMessage;

	switch (message) {
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			for (i = 0; i < MAX_CLIENT; i++)
				if (connList[i].socket == (SOCKET)wParam) {
					closesocket(connList[i].socket);
					connList[i].socket = 0;
					continue;
				}
		}

		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_ACCEPT:
		{
			connSock = accept((SOCKET)wParam, (sockaddr *)&clientAddr, &clientAddrLen);
			if (connSock == INVALID_SOCKET) {
				break;
			}
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			for (i = 0; i < MAX_CLIENT; i++)
				if (connList[i].socket == 0) {
					connList[i].socket = connSock;
					strcpy(connList[i].clientIP, clientIP);
					connList[i].clientPort = clientPort;
					connList[i].login = false;
					//requests Windows message-based notification of network events for listenSock
					WSAAsyncSelect(connList[i].socket, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
					break;
				}
			if (i == MAX_CLIENT)
				MessageBox(hWnd, L"Too many clients!", L"Notice", MB_OK);
		}
		break;

		case FD_READ:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (connList[i].socket == (SOCKET)wParam)
					break;

			ret = recv(connList[i].socket, rcvBuff, BUFF_SIZE, 0);
			if (ret > 0) {
				//process & send back to client
				rcvBuff[ret] = 0;

				//adding new received buff to message queue to process
				rcvMessage = rcvBuff;
				memset(rcvBuff, 0, BUFF_SIZE);
				messageQueue = messageQueue + rcvMessage;

				processData(messageQueue, connList[i]);
			}
		}
		break;

		case FD_CLOSE:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (connList[i].socket == (SOCKET)wParam) {
					closesocket(connList[i].socket);
					connList[i].socket = 0;
					break;
				}
		}
		break;
		}
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;

	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
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

		strcpy(buff, FAILED_LOGIN);
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
		strcpy(buff, SUCCESS_POST);
		ret = send(client.socket, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.\n", WSAGetLastError());
	}
	else {
		//save logs to file "log_20194374.txt"
		writeLog(client, data, FAILED_POST);

		//response to client
		memset(buff, 0, BUFF_SIZE);
		strcpy(buff, FAILED_POST);
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
	strcpy(buff, SUCCESS_LOGOUT);
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
	strcpy(buff, SYNTAX_ERROR);
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