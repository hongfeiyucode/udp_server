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

	printf("UDP��������\n");

	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
		

	while (true)
	{
		iResult = recvfrom(serSocket, recvbuf, DEFAULT_BUFLEN, 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if (iResult > 0)
		{
			printf("\n-----------------------------\n�ͻ��ˣ�Hello ��ʼ��������.\n���Կͻ���IP��ַ��%s �˿ڵ�ַ��%d \n\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
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

	printf("���߳̿�����\n");
	
	char * sayhello = "Hello ���������ѿ��������Է��ͣ�\n" ;
	iResult=sendto(serSocket, sayhello, DEFAULT_BUFLEN, 0, (sockaddr *)&remoteAddr, nAddrLen);
	if(iResult>0)cout << "��������Hello ���������ѿ��������Է��ͣ�" << endl;

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
			cout << "�ļ���Ϊ" << filename <<"���½��ļ���..."<< endl;


		}

		iResult = recvfrom(serSocket, recvbuf, sizeof(char *), 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if (iResult >0)
		{
			invfilelen = *((int *)recvbuf);
			cout << "���ݳ��ȵ������ֽ���Ϊ  " << invfilelen << endl;
			filelen = ntohl(invfilelen);
			cout << "���ݳ���Ϊ  " << filelen << endl;
		}
		file = fopen(filename, "wb+");
		if (file == NULL) { cout << "���ļ�ʧ�ܣ�" << endl; return -1; }

		int torecvlen = filelen;
		// �����������ݣ�ֱ���Է��ر����� 
		do
		{
			iResult = recvfrom(serSocket, recvbuf, recvbuflen, 0, (sockaddr *)&remoteAddr, &nAddrLen);

			if (iResult > 0)
			{
				cout << "����" << torecvlen << "���ֽ���Ҫ����" << endl;
				int recvlen = iResult;
				if (iResult > torecvlen)recvlen = torecvlen;
				fwrite(recvbuf, 1, recvlen, file);
				torecvlen -= recvlen;
				if (torecvlen <= 0)break;

				//���1���ɹ����յ�����
				//printf("���յ�����:  %s(%d)\n", recvbuf, iResult);

			}
			else if (iResult == 0)
			{
				//���2�����ӹر�
				printf("���ӹر�...\n");
			}
			else
			{
				//���3�����շ�������
				printf("���շ������󣡴�����: %d\n", WSAGetLastError());
				closesocket(serSocket);
				return -1;
			}
		} while (iResult > 0);

		fclose(file);


		char * sendData = "һ�����Է���˵�UDP���ݰ�\n";
		sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&remoteAddr, nAddrLen);

	}

	return 0;
}