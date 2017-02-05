#include "IOCP_Server.h"

int main(int argc, char** argv)
{
	WSADATA wsaData;
	// ������ CompletionPort�� ���� �� Handle
	HANDLE hCompletionPort;

	// �����带 ������ �� CPU�� ������ ��� �ؾ��ϹǷ� �ý��� ������ ȹ��
	SYSTEM_INFO SystemInfo;

	SOCKADDR_IN servAddr;

	// ������ ���� ����
	LPPER_IO_DATA PerIoData;

	// ���� ����
	LPPER_HANDLE_DATA PerHandleData;

	SOCKET hServSock;
	int RecvBytes;
	int Flags;

	// Load Winsock 2.2 DLL
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		ErrorHandling("WSAStartup() error : %d",WSAGetLastError());
	}

	// 1.CompletionPort ����
	// CreateCompletionPort�� ù ��°�� �ش��ϸ�, CompletionPort ������
	// ������ ������ ���ڸ� ���� �ǹ� �����̴�
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);

	// �ý��� ���� ȹ��
	// �ʿ��� �κ��� CPU�� ������ �����̸�, ������� dwNumberOfProcessors���� Ȯ�� ����
	GetSystemInfo(&SystemInfo);

	// 2.CompletionPort���� ����� �ϷḦ ��ٸ��� �����带 CPU ������ŭ ����
	// CPU ������ŭ �����带 �������ִ� �κ�
	// ������ ������ ���ڸ� ������� ����� ���̴�
	for(unsigned int i = 0; i<SystemInfo.dwNumberOfProcessors; i++)
	{
		_beginthreadex(NULL,0,CompletionThread,(LPVOID)hCompletionPort,0,NULL);
	}

	// IOCP�� ������ �ݵ�� ��ø ����� ���(OVERLAPPED)���� �����Ͽ��� ��
	hServSock = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi("6664"));

	bind(hServSock,(SOCKADDR*)&servAddr,sizeof(servAddr));
	listen(hServSock,5);

	while(true)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		hClntSock = accept(hServSock,(SOCKADDR*)&clntAddr,&addrLen);
		Log("Ŭ���̾�Ʈ�� �����Ͽ����ϴ� : %s\n",inet_ntoa(clntAddr.sin_addr));
		
		// ����� Ŭ���̾�Ʈ�� ���� ������ �����Ҵ����� ����
		PerHandleData = new PER_HANDLE_DATA;
		PerHandleData->hClntSock = hClntSock;
		memcpy(&(PerHandleData->clntAddr),&clntAddr,addrLen);

		// 3.Overlapped ���ϰ� CompletionPort�� ����
		// CreateIoCompleationPort�� �� ��° �������, ���� ����� Ŭ���̾�Ʈ�� hCompletionPort�� ����
		// �� ��° ���ڷ� Ŭ���̾�Ʈ ������ ��Ҵ� PerHandleData�� ����
		// ������ ������ ���ڸ� �մ��� ������ �ǽðڴ�
		CreateIoCompletionPort((HANDLE)hClntSock,hCompletionPort,(DWORD)PerHandleData,0);

		// ����� Ŭ���̾�Ʈ�� ���� ���۸� �����ϰ� overlapped ����ü ���� �ʱ�ȭ
		// ���� ���� ������ ���� PerIoData ���� �� �ʱ�ȭ
		PerIoData = new PER_IO_DATA;
		memset(&(PerIoData->overlapped),0,sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;

		// 4.��ø�� ������ �Է�
		WSARecv(PerHandleData->hClntSock,	// ������ �Է� ����
			&(PerIoData->wsaBuf),			// ������ �Է� ���� ������
			1,								// ������ �Է� ������ ��
			(LPDWORD)&RecvBytes,
			(LPDWORD)&Flags,
			&(PerIoData->overlapped),		// overlapped ����ü ������
			NULL);
	}
	return 0;
}

// ����� �Ϸῡ ���� �������� �ൿ �����̸�, ���� ��� ������
unsigned int __stdcall CompletionThread(LPVOID pComPort)
{
	//** �� ���� ���� ���� LPVOID�� ����ȯ�� �ʿ伺�� �ִ� ���ΰ�? �׳� HANDLE ����ϸ� �ȵǳ�?
	HANDLE hCompletionPort = (HANDLE)pComPort;

	DWORD BytesTransferred;
	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_DATA PerIoData;
	DWORD flags;

	while(true)
	{
		// 5. ������� �Ϸ�� ���� ���� ȹ��
		// ù��° ���ڷ� ���޵� hCompletionPort�� ����� ���ϵ��� �Ϸ�� ������ ���
		// ����, �Ϸ�Ǵ� ���Ϻ��� ����
		GetQueuedCompletionStatus(hCompletionPort,
			&BytesTransferred,						// ���۵� ����Ʈ ��
			(LPDWORD)&PerHandleData,
			(LPOVERLAPPED*)&PerIoData,				// overlapped ����ü ������
			INFINITE);
		// �� ��° ���ڷ� ���޵� PerHandleData�� ������� �Ϸ�� ��������,
		// hCompletionPort�� ������ �� ���ڷ� ������ Ŭ���̾�Ʈ ������ ���޵�

		// �� ��° ���ڷ� ���޵� PerIoData�� WSA Send/Recv �ÿ� ������
		// overlapped ��� ������ �����͸� ��� ���� ���

		// (LPPER_IO_DATA ����ü�� ù ��° ��� ������ overlapped
		// ���� ���� ���� �ּҴ� �����ϱ⿡ WSARecv���� overlapped�� �ּҸ� �Ѱܵ�
		// �̰����� LPPER_ID_DATA�� ��� ����)

		// EOF �߻�
		if(!BytesTransferred)
		{
			// Ŭ���̾�Ʈ ���� ���� �� ó��
			closesocket(PerHandleData->hClntSock);
			delete PerHandleData;
			delete PerIoData;

			continue;
		}
		
		// ������ ���� ���ڿ� ���� �޼��� �߰�
		PerIoData->wsaBuf.buf[BytesTransferred] = '\0';
		printf("Recv[%s]\n",PerIoData->wsaBuf.buf);

		// 6. Ŭ���̾�Ʈ�� �޼��� ����
		PerIoData->wsaBuf.len = BytesTransferred;
		WSASend(PerHandleData->hClntSock,&(PerIoData->wsaBuf),1,NULL,0,NULL,NULL);

		// ���۸� ���� ���� �ʱ�ȭ
		memset(&(PerIoData->overlapped),0,sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;

		flags = 0;
		WSARecv(PerHandleData->hClntSock,
			&(PerIoData->wsaBuf),
			1,
			NULL,
			&flags,
			&(PerIoData->overlapped),
			NULL);
	}
	return 0;
}

void Log(const char* fmt, ...)
{
	char* buf;
	int len;
	va_list args;

	va_start(args,fmt);
	len = _vscprintf(fmt,args)+1;
	buf = new char[sizeof(char)*len];
	memset(buf,0,len);
	vsprintf_s(buf,len,fmt,args);
	va_end(args);

	fputs(buf,stderr);
	fputc('\n',stderr);

	delete[] buf;
}

void ErrorHandling(const char* fmt, ...)
{
	char* buf;
	int len;
	va_list args;

	va_start(args,fmt);
	len = _vscprintf(fmt,args)+1;
	buf = new char[sizeof(char)*len];
	memset(buf,0,len);
	vsprintf_s(buf,len,fmt,args);
	va_end(args);

	fputs(buf,stderr);
	fputc('\n',stderr);

	delete[] buf;
	printf("Press any key to exit.");
	_getch();
	exit(1);
}