#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
// ���� ���� ���
#include <stdarg.h>
// _getch()�� �̿��ϱ� ���� �����, �ܼ� ����
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE		1024
#define SERVER_PORT	6664
/*
typedef struct
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct
{
	OVERLAPPED overlapped;
	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;
*/
// ���� �� ���� ������ ����
struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	SOCKET socket;
	char messageBuffer[BUFSIZE];
	int receiveBytes;
	int sendBytes;
};
/*
���� ���� ������ ����ü�� ����µ�, �� �� WSABUF�� overlapped�� �����Ѵ�
WSABUF�� WSASend, WSARecv �Լ��� ���ڷ� ���޵Ǵ� ���ۿ� ���Ǵ� ����ü�̱⿡ ���Եǰ�,
overlapped ����ü ������ �־��ִ� ������ ���� �Ϸ�� ����� ������ �� �� ����ϱ� ����
*/

// �Ϸ�� �����忡 ���� ó���� ���ִ� �Լ�
// CompletionThread
unsigned int __stdcall CompletionThread(LPVOID hCompletionPort);

//void Send(PER_HANDLE_DATA* PerHandleData,PER_IO_DATA* PerIoData);
//void Recv(PER_HANDLE_DATA* PerHandleData,PER_IO_DATA* PerIoData);

//* �Լ��� �������, ������
void CALLBACK CompRoutine(DWORD dwError,DWORD szRecvBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags);

void Log(const char* fmt, ...);
void ErrorHandling(const char* fmt, ...);