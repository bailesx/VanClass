#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
// ���� ���� ���
#include <stdarg.h>
// _getch()�� �̿��ϱ� ���� �����, �ܼ� ����
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

void ErrorHandling(const char* fmt, ...);

//** ����
class CClient
{
private:


public:
	CClient(){ };
	~CClient(){ };
};
