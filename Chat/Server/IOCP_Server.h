#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
// ���� ���� ���
#include <stdarg.h>
// _getch()�� �̿��ϱ� ���� �����, �ܼ� ����
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 1024

// Ŭ���̾�Ʈ ���� ����
typedef struct  
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// Ŭ���̾�Ʈ�κ��� ���۵� ������ ����(������ ���� ����)
typedef struct
{
	OVERLAPPED overlapped;
	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

/*
���� ���� ������ ����ü�� ����µ�, �� �� WSABUF�� overlapped�� �����Ѵ�
WSABUF�� WSASend, WSARecv �Լ��� ���ڷ� ���޵Ǵ� ���ۿ� ���Ǵ� ����ü�̱⿡ ���Եǰ�,
overlapped ����ü ������ �־��ִ� ������ ���� �Ϸ�� ����� ������ �� �� ����ϱ� ����
*/

// �Ϸ�� �����忡 ���� ó���� ���ִ� �Լ�
unsigned int __stdcall CompletionThread(LPVOID pComPort);

void Log(const char* fmt, ...);
void ErrorHandling(const char* fmt, ...);