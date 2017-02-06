#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
// 가변 인자 사용
#include <stdarg.h>
// _getch()를 이용하기 위한 헤더로, 콘솔 전용
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 1024

// 소켓 및 전송 데이터 정보
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

void Send(socketinfo* pSocketInfo);
void Recv(socketinfo* pSocketInfo);

void CALLBACK CompRoutine(DWORD dwError,DWORD szRecvBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags);

void ErrorHandling(const char* fmt, ...);

//** 예비
/*
class CClient
{
private:


public:
	CClient(){ };
	~CClient(){ };
};
*/
/*
void ErrorHandling(int errcode)
{
	LPVOID lpMsgBuf = 0;
	Formatdatainfo->pSocketInfo->buffer( 
		FORMAT_datainfo->pSocketInfo->buffer_ALLOCATE_BUFFER|
		FORMAT_datainfo->pSocketInfo->buffer_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[오류] %s", (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
*/