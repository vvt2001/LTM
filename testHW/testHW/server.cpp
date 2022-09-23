#include "stdio.h"
#include "conio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "iostream"
#include "string"
using namespace std;
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
#define ERROR_MESSAGE "Failed: String contains non-number character."
#pragma comment(lib, "Ws2_32.lib")

string process(char* data) {
	int sum=0;
	string ssum;
	for (int i = 0; i < strlen(data); i++) {
		if (data[i] < 48 || data[i]>57) {
			return ERROR_MESSAGE;
			break;
		}
		else sum += (data[i] - 48);
	}
	ssum = to_string(sum);
	return ssum;
}

int main(int argc, char* argv[])
{
	char portChar[100];
	strcpy_s(portChar, argv[1]);
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
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		printf("Error %d: Cannot associate a local address with server socket", WSAGetLastError());
	}

	//step3: bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket\n", WSAGetLastError());
		_getch();
		return 0;
	}

	//step4: Listen for request from client
	if (listen(listenSock, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN \n", WSAGetLastError());
		return 0;
	}
	printf("Server started!\n");

	//step 5: communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;
	SOCKET connSock;
	string messageQueue;

	while (1) {
		//accept request
		connSock = accept(listenSock, (sockaddr *)& clientAddr,
			&clientAddrLen);
		if (connSock == SOCKET_ERROR) {
			printf("Error %d: Cannot permit incoming connection\n", WSAGetLastError);
			return 0;
		}
		else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP,
				sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Accept incoming connection from %s: %d\n", clientIP, clientPort);
		}

		while (1) {
			//receive message from client
			ret = recv(connSock, buff, 1024, 0);
			if (ret == SOCKET_ERROR) {
				printf("Error %d: Cannot receive data\n", WSAGetLastError());
				break;
			}
			else if (ret == 0) {
				printf("Client disconnects\n");
				break;
			}

			//if received
			else {
				//process
				buff[ret] = 0;
				string tmp=buff;
				messageQueue = messageQueue + tmp;
				int end_deliPos = messageQueue.find(ENDING_DELIMITER);
				
				//if found ending delimiter -> process
				while (end_deliPos >=0 ) {
					printf("Receive from client[%s:%d] %s\n", clientIP, clientPort, buff);
					string data = messageQueue.substr(0, end_deliPos);
					string result = process(&data[0]);

					//add prefix
					if (strcmp(&result[0], ERROR_MESSAGE)==0) result = "-" + result;
					else result = "+" + result;

					strcpy_s(buff, &result[0]);

					//send back
					ret = send(connSock, buff, strlen(buff), 0);
					if (ret == SOCKET_ERROR) {
						printf("Error %d: Cannot send data\n", WSAGetLastError());
						break;
					}

					//clear processed data 
					Sleep(3000);
					messageQueue = messageQueue.substr(end_deliPos + 2, messageQueue.length()-1);

					//if there is stil processable data -> continue
					if(messageQueue!="") end_deliPos = messageQueue.find(ENDING_DELIMITER);
					else break;
				}//end while process
			}
		} //end while communicate
	}//end while accept

	  //step6: close socket
	closesocket(listenSock);
	closesocket(connSock);
	//step7: terminate winsock
	WSACleanup();
	return 0;
}
