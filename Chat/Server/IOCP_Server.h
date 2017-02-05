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

// 클라이언트 소켓 정보
typedef struct  
{
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// 클라이언트로부터 전송된 데이터 정보(소켓의 버퍼 정보)
typedef struct
{
	OVERLAPPED overlapped;
	char buffer[BUFSIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

/*
소켓 버퍼 정보를 구조체로 만드는데, 이 때 WSABUF와 overlapped를 포함한다
WSABUF는 WSASend, WSARecv 함수의 인자로 전달되는 버퍼에 사용되는 구조체이기에 포함되고,
overlapped 구조체 변수를 넣어주는 이유는 현재 완료된 입출력 정보를 얻어낼 때 사용하기 때문
*/

// 완료된 쓰레드에 관한 처리를 해주는 함수
unsigned int __stdcall CompletionThread(LPVOID pComPort);

void Log(const char* fmt, ...);
void ErrorHandling(const char* fmt, ...);