#include "stdafx.h"  
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable:4996)
#include <stdio.h>  
#include <winsock2.h>  
#include <iostream>
using namespace std;
#define DEFAULT_BUFLEN 1024
#pragma comment(lib, "ws2_32.lib")  

DWORD WINAPI AnewThread(LPVOID lParam);


int main(int argc, char* argv[])
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);

	int iResult = 0;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind error !");
		closesocket(serSocket);
		return 0;
	}

	printf("UDP服务开启！\n");

	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
		

	while (true)
	{
		iResult = recvfrom(serSocket, recvbuf, DEFAULT_BUFLEN, 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if (iResult > 0)
		{
			printf("\n-----------------------------\n客户端：Hello 开始传送数据.\n来自客户端IP地址：%s 端口地址：%d \n\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
			DWORD dwThreadID;
			CreateThread(NULL, NULL, AnewThread, &remoteAddr, 0, &dwThreadID);
			if(!AnewThread)CloseHandle(AnewThread);

		}
		
	}
	closesocket(serSocket);
	WSACleanup();
	return 0;
}



DWORD WINAPI AnewThread(LPVOID lParam)
{
	int iResult = 0;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	sockaddr_in remoteAddr  = *((sockaddr_in *)lParam);
	int nAddrLen = sizeof(remoteAddr);
	SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}
	
	u_short port = htons(8889);
	sockaddr_in  serAddr;	
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = port;
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	while (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		port++;
		serAddr.sin_port = htons(port);
	}

	printf("新线程开启！\n");
	
	char * sayhello = "Hello 数据连接已开启，可以发送！\n" ;
	iResult=sendto(serSocket, sayhello, DEFAULT_BUFLEN, 0, (sockaddr *)&remoteAddr, nAddrLen);
	if(iResult>0)cout << "服务器：Hello 数据连接已开启，可以发送！" << endl;

	while (true)
	{
		FILE *file = NULL;
		char filename[100];
		int namelen = 0;
		int filelen, invfilelen;

		iResult = recvfrom(serSocket, recvbuf, DEFAULT_BUFLEN, 0, (sockaddr *)&remoteAddr, &nAddrLen);

		if (iResult > 0)
		{
			int pos;
			for (int i = 0; i < strlen(recvbuf); i++)
				if (recvbuf[i] == '\\')pos = i;
			for (int i = pos + 1; i < strlen(recvbuf); i++)
				filename[i - pos - 1] = recvbuf[i];
			cout << strlen(recvbuf) - pos-1;
			filename[strlen(recvbuf) - pos-1] = '\0';
			cout << "文件名为" << filename <<"，新建文件中..."<< endl;


		}

		iResult = recvfrom(serSocket, recvbuf, sizeof(char *), 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if (iResult >0)
		{
			invfilelen = *((int *)recvbuf);
			cout << "数据长度的网络字节序为  " << invfilelen << endl;
			filelen = ntohl(invfilelen);
			cout << "数据长度为  " << filelen << endl;
		}
		file = fopen(filename, "wb+");
		if (file == NULL) { cout << "打开文件失败！" << endl; return -1; }

		int torecvlen = filelen;
		// 持续接收数据，直到对方关闭连接 
		do
		{
			iResult = recvfrom(serSocket, recvbuf, recvbuflen, 0, (sockaddr *)&remoteAddr, &nAddrLen);

			if (iResult > 0)
			{
				cout << "还有" << torecvlen << "个字节需要接收" << endl;
				int recvlen = iResult;
				if (iResult > torecvlen)recvlen = torecvlen;
				fwrite(recvbuf, 1, recvlen, file);
				torecvlen -= recvlen;
				if (torecvlen <= 0)break;

				//情况1：成功接收到数据
				//printf("接收到数据:  %s(%d)\n", recvbuf, iResult);

			}
			else if (iResult == 0)
			{
				//情况2：连接关闭
				printf("连接关闭...\n");
			}
			else
			{
				//情况3：接收发生错误
				printf("接收发生错误！错误编号: %d\n", WSAGetLastError());
				closesocket(serSocket);
				return -1;
			}
		} while (iResult > 0);

		fclose(file);


		char * sendData = "一个来自服务端的UDP数据包\n";
		sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&remoteAddr, nAddrLen);

	}

	return 0;
}