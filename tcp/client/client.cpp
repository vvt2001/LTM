#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <string>
#include <string.h>
#include <vector>
#include <chrono>
#include <ctime>
#include <fstream>
#include "process.h"
#pragma comment(lib, "Ws2_32.lib")
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"
#define SPLITING_DELIMITER_1 "\t\n"
#define SPLITING_DELIMITER_2 "\f\n"
using namespace std;
string login_id = "", room_id = "";

unsigned __stdcall receiveThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;

	SOCKET connectedSocket = (SOCKET)param;
	while (1) {
		ret = recv(connectedSocket, buff, BUFF_SIZE, 0);
		buff[ret] = 0;
		string message = buff;
		string code = message.substr(0, 2);
		string payload = message.substr(2, message.length() - 4);
		int code_in_int = stoi(code);
		switch (code_in_int) {
		case 30: {
			while (payload != "") {
				int delimiter_index = payload.find("\t\n");
				string room_info = payload.substr(0, delimiter_index);

				int spliting_index = room_info.find("\f\n");
				string room_id = room_info.substr(0, spliting_index);
				room_info = room_info.substr(spliting_index + 2);
				cout << "Room id: " << room_id << endl;

				spliting_index = room_info.find("\f\n");
				string item_name = room_info.substr(0, spliting_index);
				room_info = room_info.substr(spliting_index + 2);
				cout << "item name: " << item_name << endl;

				spliting_index = room_info.find("\f\n");
				string item_description = room_info.substr(0, spliting_index);
				room_info = room_info.substr(spliting_index + 2);
				cout << "Room description: " << item_description << endl;

				spliting_index = room_info.find("\f\n");
				string current_price = room_info.substr(0, spliting_index);
				room_info = room_info.substr(spliting_index + 2);
				cout << "current price: " << current_price << endl;

				spliting_index = room_info.find("\f\n");
				string buy_immediately_price = room_info.substr(0, spliting_index);
				room_info = room_info.substr(spliting_index + 2);
				cout << "buy immediately price: " << buy_immediately_price << endl;
				cout << "=========================" << endl;
				payload = payload.substr(delimiter_index + 2);
			}
			break;
		}
		case 10:
		{
			login_id = payload;
			cout << "Success login. User id: " << payload << endl;
			break;
		}
		case 11:
			cout << "Failed login." << endl;
			break;
		case 40: {
			room_id = payload;
			cout << "Joined room. Room id: " << payload << endl;
			break;
		}
		case 41: {
			string owner = payload.substr(0, payload.find(SPLITING_DELIMITER_1));
			string final_price = payload.substr(payload.find(SPLITING_DELIMITER_1) + 2);
			cout << "Room closed. Owner: " << owner << ". " << "Final price: " << final_price << ". " << endl;
			break;
		}
		case 83:
			cout << "Room ID not found!" << endl;
			break;
		case 50:
			cout << "Bid success." << endl;
			break;
		case 51:
			cout << "Bid value lower than current price." << endl;
			break;
		case 52:
			cout << "Room creator can't bid." << endl;
			break;
		case 60:
			cout << "Buy immediately success." << endl;
			break;
		case 61:
			cout << "Failed. Item already sold." << endl;
			break;
		case 62:
			cout << "Room creator can't buy." << endl;
			break;
		case 70:
			cout << "Create room success. Room ID: " << payload << ". " << endl;
			break;
		case 71:
			cout << "Invalid information." << endl;
			break;
		case 80:
			cout << "Item sold. Owner ID: " << payload << ". " << endl;
			break;
		case 81:
			cout << "Times left: " << payload << " min. " << endl;
			break;
		case 82:
			cout << "New price: " << payload << ". " << endl;
			break;
		case 90: {
			room_id = "";
			cout << "Left room." << endl;
			break;
		}
		}

		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
		else {
			buff[ret] = 0;
			//printf("buff: %s", buff);
		}
	}

	closesocket(connectedSocket);
	return 0;
}
unsigned __stdcall sendThread(void *param) {
	char buff[BUFF_SIZE];
	int ret;
	string t;

	SOCKET connectedSocket = (SOCKET)param;
	while (1) {
		//Send message
		cout << "======= Choose your option ========" << endl;
		cout << "1. Login " << endl;
		cout << "2. create room" << endl;
		cout << "3. join room" << endl;
		cout << "4. show room" << endl;
		cout << "5. bid a new price" << endl;
		cout << "6. buy immediately" << endl;
		cout << "7. leave room" << endl;
		cout << "Your option: ";
		cin >> t;
		int messageLen = strlen(buff);
		if (messageLen == 0) break;
		if (t == "1") {
			if (login_id != "") {
				cout << "You have logged in. Please log out first." << endl;
				messageLen = -1;
			}
			else {
				string email, password;
				cout << "Enter your email:" << endl;
				cin >> email;
				cout << "Enter your password" << endl;
				cin >> password;
				string message = "LOGIN_" + email + "\t\n" + password + "\r\n";
				strcpy_s(buff, message.length() + 1, &message[0]);
				messageLen = message.length();
			}
		}
		else if (t == "2") {
			if (login_id == "") {
				cout << "You have not logged in." << endl;
				messageLen = -1;
			}
			else {
				string user_id, room_id, item_name, item_description, starting_price, buy_immediately_price;
				cout << "Enter your item name" << endl;
				cin >> item_name;
				cout << "Enter your item description:" << endl;
				cin >> item_description;
				cout << "Enter starting price" << endl;
				cin >> starting_price;
				cout << "Enter buy immediately price" << endl;
				cin >> buy_immediately_price;

				string message = "CREATE" + login_id + "\t\n" + item_name + "\t\n" + item_description + "\t\n" + starting_price + "\t\n" + buy_immediately_price + "\r\n";
				strcpy_s(buff, message.length() + 1, &message[0]);
				messageLen = message.length();
			}
		}
		else if (t == "3") {
			if (login_id == "") {
				cout << "You have not logged in." << endl;
				messageLen = -1;
			}
			else if (room_id != "") {
				cout << "You have joined room " << room_id << "Please leave current room to join a new room" << endl;
				messageLen = -1;

			}
			else {
				// join room
				string user_id, joined_room_id;
				cout << "Enter your room id" << endl;
				cin >> joined_room_id;
				string message = "JOIN__" + login_id + "\t\n" + joined_room_id + "\r\n";
				strcpy_s(buff, message.length() + 1, &message[0]);
				messageLen = message.length();
			}
		}
		else if (t == "4") {
			strcpy_s(buff, "SHOW__\r\n"); // length 10
			messageLen = 10;
		}
		else if (t == "5") {
			if (login_id == "") {
				cout << "You have not logged in." << endl;
				messageLen = -1;
			}
			else if (room_id == "") {
				cout << "You have not joined any room" << endl;
				messageLen = -1;

			}
			else {
				string price;
				cout << "Enter your price" << endl;
				cin >> price;
				string message = "BID___" + price + "\t\n" + login_id + "\t\n" + room_id + "\r\n";
				strcpy_s(buff, message.length() + 1, &message[0]);
				messageLen = message.length();
			}
		}
		else if (t == "6") {
			if (login_id == "") {
				cout << "You have not logged in." << endl;
				messageLen = -1;
			}
			else if (room_id == "") {
				cout << "You have not joined any room" << endl;
				messageLen = -1;
			}
			else {
				string message = "BUYNOW" + login_id + "\t\n" + room_id + "\r\n";
				strcpy_s(buff, message.length() + 1, &message[0]);
				messageLen = message.length();
			}
		}
		else if (t == "7") {
			if (login_id == "") {
				cout << "You have not logged in." << endl;
				messageLen = -1;
			}
			else if (room_id == "") {
				cout << "You have not joined any room" << endl;
				messageLen = -1;
			}
			else {
				// leave room
				string message = "LEAVE_" + login_id + "\t\n" + room_id + "\r\n";
				strcpy_s(buff, message.length() + 1, &message[0]);
				messageLen = message.length();
			}
		}
		else {
			cout << "Wrong choice. Please pick a number for 1-7" << endl;
			messageLen = -1;
		}
		if (messageLen != -1) {
			ret = send(connectedSocket, buff, messageLen, 0);
			if (ret == SOCKET_ERROR)
				printf("Error %d", WSAGetLastError());
		}
		Sleep(200);
	}

	closesocket(connectedSocket);
	return 0;
}
int main(int argc, char* argv[])
{
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


	//step3: specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

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
	_beginthreadex(0, 0, sendThread, (void *)client, 0, 0);
	_beginthreadex(0, 0, receiveThread, (void *)client, 0, 0);
	while (1) {
		//do nothing

	};
	//step 6: close socket
	closesocket(client);
	//step7: terminate winsock
	WSACleanup();
	return 0;
}