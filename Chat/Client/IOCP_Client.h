#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
// 가변 인자 사용
#include <stdarg.h>
// _getch()를 이용하기 위한 헤더로, 콘솔 전용
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

void ErrorHandling(const char* fmt, ...);

//** 예비
class CClient
{
private:


public:
	CClient(){ };
	~CClient(){ };
};
