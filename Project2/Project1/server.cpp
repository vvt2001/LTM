#include "stdio.h"
#include "conio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "string.h"
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

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
	SOCKET server;
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	//step3: bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot bind this address.", WSAGetLastError());
		_getch();
		return 0;
	}
	printf("Server started!\n");

	//step4: communicate with client
	while (1) {
		sockaddr_in clientAddr;
		char buff[BUFF_SIZE];
		char clientIP[INET_ADDRSTRLEN];
		int ret, clientAddrLen = sizeof(clientAddr), clientPort;


		//receive message
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot receive data.", WSAGetLastError());
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			int messageLen = strlen(buff);
			int lastChar = messageLen - 1;//bien xac dinh vi tri ky tu danh dau
			char a = buff[lastChar];
			buff[lastChar] = '\0';	//xoa ky tu danh dau o cuoi de thuc hien phan giai
			buff[ret] = 0;
			messageLen = strlen(buff);
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("Receive from client[%s:%d] %s\n", clientIP, clientPort, buff);


			//phan giai thuan
			if (a == '1') {
				addrinfo *result; //pointer to the linked-list
								  //containing information about the host
				int rc;
				sockaddr_in *address;
				addrinfo hints;
				memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_INET;
				rc = getaddrinfo(buff, NULL, &hints, &result);
				// Get the address info & send back
				char ipStr[INET_ADDRSTRLEN];
				if (rc == 0) {
					while (result != NULL) {
						address = (struct sockaddr_in *) result->ai_addr;
						inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
						strcpy_s(buff, ipStr);
						ret = sendto(server, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
						if (ret == SOCKET_ERROR)
							printf("Error %d: Cannot send data\n", WSAGetLastError());
						result = result->ai_next;
					}
					if (result == NULL) {
						strcpy_s(buff, "end");
						ret = sendto(server, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
					}
				}
				else {
					strcpy_s(buff, "Not found information");
					ret = sendto(server, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
				}
				// free linked-list
				freeaddrinfo(result);
			}//ket thuc phan giai thuan


			 //phan giai nguoc
			if (a == '2') {
				struct sockaddr_in addr;
				memset(&addr, 0, sizeof(sockaddr_in));
				int ret1, ret2;
				char hostname[NI_MAXHOST];
				char servInfo[NI_MAXSERV];
				addr.sin_family = AF_INET;

				ret1 = inet_pton(AF_INET, buff, &addr.sin_addr);

				if (ret1 != 1) {
					strcpy_s(buff, "Not found information");
					sendto(server, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
				}
				else {
					ret2 = getnameinfo((struct sockaddr *) &addr,
					sizeof(struct sockaddr),
					hostname, NI_MAXHOST,
					servInfo, NI_MAXSERV, NI_NUMERICSERV);
					if (ret2 == 0) {
						strcpy_s(buff, hostname);
						sendto(server, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
					}
					else {
						strcpy_s(buff, "Not found information");
						sendto(server, buff, strlen(buff), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
					}
				}
			}//ket thuc phan giai nguoc
		}
	}//end while

	//step5: close socket
	closesocket(server);
	//step6: terminate winsock
	WSACleanup();
	return 0;
}