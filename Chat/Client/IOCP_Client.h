#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
// 가변 인자 사용
#include <stdarg.h>
// _getch()를 이용하기 위한 헤더로, 콘솔 전용
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE		1024
#define SERVER_IP	"127.0.0.1"
#define SERVER_PORT	6664

// 소켓 및 전송 데이터 정보
typedef struct SocketInfo
{
	SOCKET hSocket;
	SOCKADDR_IN compAddr;
} socketinfo;

typedef struct DataInfo
{
	WSAOVERLAPPED overlapped;

	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} datainfo;

void Send(SOCKET &hSocket,WSABUF &dataBuf,char* message,WSAOVERLAPPED &overlapped);
void Recv(SOCKET &hSocket,WSABUF &dataBuf,char* message,WSAOVERLAPPED &overlapped);
//void Send(socketinfo* pSocketInfo);
//void Recv(socketinfo* pSocketInfo);

//* 함수만 만들었지, 무쓸모
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