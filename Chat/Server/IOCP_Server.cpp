#include "IOCP_Server.h"

void main(int argc, char** argv)
{
	// Winsock Start - windock.dll �ε�
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		ErrorHandling("ERROR - Failed WSAStartup() : %d", WSAGetLastError());
	}

	// IOCP�� ������ �ݵ�� ��ø ����� ���(OVERLAPPED)���� �����Ͽ��� ��
	SOCKET hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET)
	{
		WSACleanup();
		ErrorHandling("ERROR - Failed WSASocket() : %d", WSAGetLastError());
	}

	// ���� ���� ��ü ����
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		closesocket(hServSock);
		WSACleanup();
		ErrorHandling("ERROR - Failed bind() : %d", WSAGetLastError());
	}
	if(listen(hServSock, 5) == SOCKET_ERROR)
	{
		closesocket(hServSock);
		WSACleanup();
		ErrorHandling("ERROR - Failed listen() : %d", WSAGetLastError());
	}

	// �Ϸ����� ó���ϴ� ��ü(CP : Completion Port) ����
	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// �����带 ������ �� CPU�� ������ ����ؾ� �ϹǷ� �ý��� ������ ȹ��
	// �ʿ��� �κ��� CPU�� ������ �����̸�, ������� dwNumberOfProcessors���� Ȯ�� ����
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	int threadCount = SystemInfo.dwNumberOfProcessors * 2;
	//** �� ������ ID�� �뵵�� Ȯ���ϸ� �̿� ��ġ�� ���� ��
	unsigned threadID;
	// ������ �ڵ鷯
	HANDLE *hThread = new HANDLE[threadCount];
	for(int i=0; i<threadCount; i++)
	{
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, CompletionThread, (LPVOID)hIOCP, 0, &threadID);
	}

	SOCKADDR_IN clntAddr;
	int addrLen = sizeof(clntAddr);
	memset(&clntAddr, 0, addrLen);
	SOCKET clientSocket;
	SOCKETINFO *socketInfo;
	DWORD receiveBytes;
	DWORD flag;
	
	while(true)
	{
		clientSocket = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);
		if(clientSocket == INVALID_SOCKET)
		{
			Log("ERROR - Failed accept() : %d", WSAGetLastError());
		}

		socketInfo = new SOCKETINFO;
		memset((void *)socketInfo,0x00,sizeof(SOCKETINFO));
		socketInfo->socket = clientSocket;
		socketInfo->receiveBytes = 0;
		socketInfo->sendBytes = 0;
		socketInfo->dataBuffer.len = BUFSIZE;
		socketInfo->dataBuffer.buf = socketInfo->messageBuffer;
		flag = 0;

		Log("Ŭ���̾�Ʈ�� �����Ͽ����ϴ� : %s\n",inet_ntoa(clntAddr.sin_addr));

		// CreateIoCompleationPort�� �� ��° �������, ���� ����� Ŭ���̾�Ʈ(overlapped)�� hIOCP�� ����
		hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (DWORD)socketInfo, 0);

		// ��ø ������ �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ��ش�.
		if (WSARecv(socketInfo->socket, &socketInfo->dataBuffer, 1, &receiveBytes, &flag, &(socketInfo->overlapped), NULL))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				closesocket(hServSock);
				WSACleanup();
				ErrorHandling("ERROR - Failed WSARecv() : %d", WSAGetLastError());
			}
		}
	}

	//** ��� Ŭ���̾�Ʈ ���� �Լ� �� �߰���
	closesocket(hServSock);
	WSACleanup();
}

unsigned int __stdcall CompletionThread(LPVOID hIOCP)
{
	HANDLE threadHandler = (HANDLE *)hIOCP;
	DWORD recvBytes;
	DWORD sendBytes;
	DWORD completionKey;
	DWORD flags;
	SOCKETINFO *eventSocket;

	while(1)
	{
		if(GetQueuedCompletionStatus(threadHandler, &recvBytes, &completionKey, (LPOVERLAPPED*)&eventSocket, INFINITE) == 0)
		{
			printf("ERROR - Failed GetQueuedCompletionStatus() : %d", WSAGetLastError());
			closesocket(eventSocket->socket);
			delete eventSocket;
		}

		eventSocket->dataBuffer.len = recvBytes;

		if(recvBytes == 0)
		{
			closesocket(eventSocket->socket);
			delete eventSocket;

			continue;
		}
		else
		{
			printf("TRACE - Receive message : %s (%d bytes)\n", eventSocket->dataBuffer.buf, eventSocket->dataBuffer.len);

			if (WSASend(eventSocket->socket, &(eventSocket->dataBuffer), 1, &sendBytes, 0, NULL, NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
				}
			}

			printf("TRACE - Send message : %s (%d bytes)\n", eventSocket->dataBuffer.buf, eventSocket->dataBuffer.len);

			memset(eventSocket->messageBuffer, 0x00, BUFSIZE);
			eventSocket->receiveBytes = 0;
			eventSocket->sendBytes = 0;
			eventSocket->dataBuffer.len = BUFSIZE;
			eventSocket->dataBuffer.buf = eventSocket->messageBuffer;
			flags = 0;

			if (WSARecv(eventSocket->socket, &(eventSocket->dataBuffer), 1, &recvBytes, &flags, &eventSocket->overlapped, NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
				}
			}
		}
	}

	return 0;
}

//void Send(PER_HANDLE_DATA* PerHandleData,PER_IO_DATA* PerIoData)
//{
//	int sendBytes;
//	if(WSASend(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, NULL, 0, NULL, NULL)==SOCKET_ERROR)
//	{
//		if(WSAGetLastError() == WSA_IO_PENDING)
//		{
//			//* ���� 2��, ��� ����ν�� ���� ������ - ���� ���� �ʿ�
//			// WSA �̺�Ʈ�� ��ٸ���, ���ۿϷḦ ��ٸ���.
//			WSAWaitForMultipleEvents( 1, &(PerIoData->overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );
//
//			// ������� �޾ƿ´�.(��� 10014 ������ �߻��ϹǷ� ����Դµ�)
//			WSAGetOverlappedResult( PerHandleData->hClntSock, &(PerIoData->overlapped), (LPDWORD)&sendBytes, FALSE, NULL );
//		}
//		else
//		{
//			ErrorHandling("WSASend() error : %d",WSAGetLastError());
//		}
//	}
//	printf("Send[%s]\n",PerIoData->wsaBuf.buf);
//}
//
//void Recv(PER_HANDLE_DATA* PerHandleData,PER_IO_DATA* PerIoData)
//{
//	int recvBytes;
//	int flags = 0;
//
//	// �����͸� �ޱ� ���� ���۸� ���� ���� �ʱ�ȭ
//	memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
//	PerIoData->wsaBuf.len = BUFSIZE;
//	PerIoData->wsaBuf.buf = PerIoData->buffer;
//
//	if(WSARecv(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, (LPDWORD)&recvBytes, (LPDWORD)&flags, &(PerIoData->overlapped), NULL)==SOCKET_ERROR)
//	{
//		if(WSAGetLastError() == WSA_IO_PENDING)
//		{
//			//* ���� 2��, ��� ����ν�� ���� ������ - ���� ���� �ʿ�
//			// WSA �̺�Ʈ�� ��ٸ���, ���ۿϷḦ ��ٸ���.
//			WSAWaitForMultipleEvents( 1, &(PerIoData->overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );
//
//			// ������� �޾ƿ´�.(��� 10014 ������ �߻��ϹǷ� ����Դµ�)
//			WSAGetOverlappedResult( PerHandleData->hClntSock, &(PerIoData->overlapped), (LPDWORD)&recvBytes, FALSE, NULL );
//		}
//		else
//		{
//			ErrorHandling("WSARecv() error : %d",WSAGetLastError());
//		}
//	}
//	printf_s("Recv[%s]\n",PerIoData->wsaBuf.buf);
//}

//* �Լ��� �������, ������
void CALLBACK CompRoutine(DWORD dwError,DWORD szRecvBytes,LPWSAOVERLAPPED lpOverlapped,DWORD flags)
{
	//SocketInfo *pSocketInfo = (SocketInfo *)lpOverlapped;
	if(dwError != 0)
	{
	}
	else
	{
		//pSocketInfo->recvbytes = szRecvBytes;
		//printf("Received message: %s \n",pSocketInfo->buffer);
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