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

// ���� �� ���� ������ ����
typedef struct SocketInfo
{
	WSAOVERLAPPED overlapped;
	SOCKET hSocket;
	SOCKADDR_IN compAddr;

	char buffer[BUFSIZE];
	WSABUF wsaBuf;
	int sendbytes;
	int recvbytes;
} socketinfo;

/*
���� ���� ������ ����ü�� ����µ�, �� �� WSABUF�� overlapped�� �����Ѵ�
WSABUF�� WSASend, WSARecv �Լ��� ���ڷ� ���޵Ǵ� ���ۿ� ���Ǵ� ����ü�̱⿡ ���Եǰ�,
overlapped ����ü ������ �־��ִ� ������ ���� �Ϸ�� ����� ������ �� �� ����ϱ� ����
*/

// �Ϸ�� �����忡 ���� ó���� ���ִ� �Լ�
unsigned int __stdcall CompletionThread(LPVOID pComPort);

void Send(socketinfo* pSocketInfo);
void Recv(socketinfo* pSocketInfo);

void CALLBACK CompRoutine(DWORD dwError,DWORD szRecvBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags);

void Log(const char* fmt, ...);
void ErrorHandling(const char* fmt, ...);