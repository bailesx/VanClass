#include "IOCP_Server.h"

void main(int argc, char** argv)
{
	WSADATA wsaData;
	// CompletionPort�� Handle
	HANDLE hCompletionPort;
	// �����带 ������ �� CPU�� ������ ����ؾ� �ϹǷ� �ý��� ������ ȹ��
	SYSTEM_INFO SystemInfo;

	// ���� ���� ����
	SOCKET hServSock;
	SOCKADDR_IN servAddr;

	// Ŭ���̾�Ʈ ���� ����
	socketinfo* pSocketInfo;

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
	unsigned int iNumThreads = SystemInfo.dwNumberOfProcessors * 2;

	// 2.CompletionPort���� ����� �ϷḦ ��ٸ��� �����带 CPU�� 2��� ����
	// ������ ������ ���ڸ� ������� ����� ���̴�
	for(unsigned int i = 0; i<iNumThreads; i++)
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
		printf("//**�����Լ��ݺ��� ����\n");
		SOCKET hSocket;
		SOCKADDR_IN compAddr;
		int addrLen = sizeof(compAddr);

		hSocket = accept(hServSock,(SOCKADDR*)&compAddr,&addrLen);
		Log("Ŭ���̾�Ʈ�� �����Ͽ����ϴ� : %s\n",inet_ntoa(compAddr.sin_addr));
		
		// ����� Ŭ���̾�Ʈ�� ���� ������ �����Ҵ����� ����
		pSocketInfo = new SocketInfo;
		pSocketInfo->hSocket = hSocket;
		memcpy(&(pSocketInfo->compAddr),&compAddr,addrLen);

		// 3.Overlapped ���ϰ� CompletionPort�� ����
		// CreateIoCompleationPort�� �� ��° �������, ���� ����� Ŭ���̾�Ʈ�� hCompletionPort�� ����
		// �� ��° ���ڷ� Ŭ���̾�Ʈ ������ ��Ҵ� pSocketInfo�� ����
		// ������ ������ ���ڸ� �մ��� ������ �ǽðڴ�
		CreateIoCompletionPort((HANDLE)pSocketInfo->hSocket,hCompletionPort,(DWORD)pSocketInfo,0);
		
		// 4.��ø�� ������ �Է�?
		Recv(pSocketInfo);
		printf("//**�����Լ��ݺ��� ���ú�\n");
	}

	//** ��� Ŭ���̾�Ʈ ���� �Լ� �� �߰���
	closesocket(hServSock);
	WSACleanup();
}

// ����� �Ϸῡ ���� �������� �ൿ �����̸�, ���� ��� ������... �Ƹ�...? switch���� �̿��ϸ�...
unsigned int __stdcall CompletionThread(HANDLE hCompletionPort)
{
	DWORD BytesTransferred;
	socketinfo* pSocketInfo;

	while(true)
	{
		printf("//**�������Լ��ݺ��� ����\n");
		// 5. ������� �Ϸ�� ���� ���� ȹ��
		// ù��° ���ڷ� ���޵� hCompletionPort�� ����� ���ϵ��� �Ϸ�� ������ ���
		// ����, �Ϸ�Ǵ� ���Ϻ��� ����
		GetQueuedCompletionStatus(hCompletionPort,
			&BytesTransferred,						// ���۵� ����Ʈ ��
			(LPDWORD)&pSocketInfo,
			(LPOVERLAPPED*)&pSocketInfo,				// overlapped ����ü ������
			INFINITE);
		// �� ��° ���ڷ� ���޵� pSocketInfo�� ������� �Ϸ�� ��������,
		// hCompletionPort�� ������ �� ���ڷ� ������ Ŭ���̾�Ʈ ������ ���޵�

		// �� ��° ���ڷ� ���޵� pSocketInfo�� WSA Send/Recv �ÿ� ������
		// overlapped ��� ������ �����͸� ��� ���� ���

		// (datainfo ����ü�� ù ��° ��� ������ overlapped
		// ���� ���� ���� �ּҴ� �����ϱ⿡ WSARecv���� overlapped�� �ּҸ� �Ѱܵ�
		// �̰����� LPPER_ID_DATA�� ��� ����)

		// EOF �߻�
		if(!BytesTransferred)
		{
			// Ŭ���̾�Ʈ ���� ���� �� ó��
			closesocket(pSocketInfo->hSocket);
			delete pSocketInfo;
			delete pSocketInfo;

			continue;
		}
		
		// ������ ���� ���ڿ� ���� �޼��� �߰�
		pSocketInfo->wsaBuf.buf[BytesTransferred] = '\0';
		printf("Recv[%s]\n",pSocketInfo->wsaBuf.buf);

		// 6. Ŭ���̾�Ʈ�� �޼��� ����
		pSocketInfo->wsaBuf.len = BytesTransferred;
		Send(pSocketInfo);
		printf("//**�������Լ��ݺ��� ����\n");
		
		Recv(pSocketInfo);
		printf("//**�������Լ��ݺ��� ���ú�\n");
	}
	return 0;
}

void Send(socketinfo* pSocketInfo)
{
	if(WSASend(pSocketInfo->hSocket,&(pSocketInfo->wsaBuf),1,(LPDWORD)&pSocketInfo->sendbytes,0,&(pSocketInfo->overlapped),NULL)==SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			ErrorHandling("WSASend() error : %d",WSAGetLastError());
		}
	}
}

void Recv(socketinfo* pSocketInfo)
{
	int flags = 0;
	
	// ���۸� ���� ���� �ʱ�ȭ
	memset(&(pSocketInfo->overlapped),0,sizeof(OVERLAPPED));
	pSocketInfo->wsaBuf.len = BUFSIZE;
	pSocketInfo->wsaBuf.buf = pSocketInfo->buffer;

	if(WSARecv(pSocketInfo->hSocket,&(pSocketInfo->wsaBuf),1,(LPDWORD)&pSocketInfo->recvbytes,(LPDWORD)&flags,&(pSocketInfo->overlapped),CompRoutine)==SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSAGetLastError() : %d",WSAGetLastError());
			//ErrorHandling("WSARecv() error : %d",WSAGetLastError());
		}
	}
}

void CALLBACK CompRoutine(DWORD dwError,DWORD szRecvBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags)
{
	SocketInfo *pSocketInfo = (SocketInfo *)lpOverlapped;
	if(dwError != 0)
	{
		ErrorHandling("CompRoutine() error");
	}
	else
	{
		pSocketInfo->recvbytes = szRecvBytes;
		printf("Received message: %s \n",pSocketInfo->buffer);
	}
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