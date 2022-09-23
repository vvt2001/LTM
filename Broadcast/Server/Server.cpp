// NonBlockingReceiver.cpp : Defines the entry point for the console application.
//

#include <stdio.h>      /* for printf(), fprintf() */
#include <winsock.h>    /* for socket(),... */
#include <stdlib.h>     /* for exit() */
#pragma comment(lib, "Ws2_32.lib")

#define MAXRECVSTRING 255  /* Longest string to receive */

void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char* argv[])
{
	int sock;                         /* Socket */
	struct sockaddr_in broadcastAddr; /* Broadcast Address */
	unsigned short broadcastPort;     /* Port */
	char recvString[MAXRECVSTRING + 1]; /* Buffer for received string */
	int recvStringLen;                /* Length of received string */
	WSADATA wsaData;                  /* Structure for WinSock setup communication */
	unsigned long nonblocking = 1;    /* Flag to make socket nonblocking */

	if (argc != 2)    /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Broadcast Port>\n", argv[0]);
		exit(1);
	}

	broadcastPort = atoi(argv[1]);   /* first arg: broadcast port */

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
	{
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}

	/* Create a best-effort datagram socket using UDP */
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	/* Set the socket to nonblocking */
	if (ioctlsocket(sock, FIONBIO, &nonblocking) != 0)
		DieWithError("ioctlsocket() failed");

	/* Construct bind structure */
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
	broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
	broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

														/* Bind to the broadcast port */
	if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
		DieWithError("bind() failed");

	/* Receive a single datagram from the server */
	for (;;)
	{
		if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				DieWithError("recvfrom() failed");
			else
			{
				printf("Still have not received packet...Waiting and then trying again\n");
				Sleep(2000);  /* Sleep for 2 milliseconds */
			}
		}
		else
			break;
	}

	recvString[recvStringLen] = '\0';
	printf("Received: %s\n", recvString);    /* Print the received string */

	closesocket(sock);
	WSACleanup();   /* Cleanup Winsock */
	return 0;
}

void DieWithError(char *errorMessage) {
	printf(errorMessage);
}