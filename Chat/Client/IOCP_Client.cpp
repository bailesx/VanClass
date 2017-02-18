#include "IOCP_Client.h"

void main()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		ErrorHandling("ERROR - Failed WSAStartup() : %d",WSAGetLastError());
	}

	// ���� ����
	SOCKET hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(hSocket == INVALID_SOCKET)
	{
		WSACleanup();
		ErrorHandling("ERROR - Failed WSASocket() : %d",WSAGetLastError());
	}

	// ���� ���� ��ü ����
	SOCKADDR_IN servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVER_PORT);
	servAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if(connect(hSocket,(SOCKADDR*)&servAddr,sizeof(servAddr)) == SOCKET_ERROR)
	{
		closesocket(hSocket);
		WSACleanup();
		ErrorHandling("ERROR - Failed connect() : %d",WSAGetLastError());
	}

	/*
	// ����ü�� �̺�Ʈ �ڵ� �����ؼ� ����
	WSAEVENT event = WSACreateEvent();
	WSAOVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = event;
	*/

	printf("���ӿ� �����Ͽ����ϴ�!\n");
	printf("ä�� �濡 ���� ���� ȯ���մϴ�!\n");
	printf("������ -help�Դϴ�.\n");
	printf("�޼����� �Է��ϼ���.\n");

	WSABUF dataBuf;
	int flags = 0;

	while(true)
	{
		char message[BUFSIZE] = {0,};
		int i, bufferLen;
		for (i = 0; 1; i++)
		{
			message[i] = getchar();
			if (message[i] == '\n')
			{
				message[i++] = '\0';
				break;
			}
		}
		bufferLen = i;

		// �Է� �޼����� exit��� ����
		if(!strcmp(message,"exit")) break;

		/* Ŭ���̾�Ʈ���� IOCP ������ ������ �Ʒ� ���� ����(������ ����)
		socketInfo = (struct SOCKETINFO *)malloc(sizeof(struct SOCKETINFO));
		memset((void *)socketInfo, 0x00, sizeof(struct SOCKETINFO));        
		socketInfo->dataBuffer.len = bufferLen;
		socketInfo->dataBuffer.buf = messageBuffer;
		*/

		// 3-1. ������ ����
		int sendBytes = send(hSocket,message,bufferLen,0);
		if (sendBytes > 0)
		{
			printf("TRACE - Send message : %s (%d bytes)\n",message,sendBytes);
			// 3-2. ������ �б�
			int receiveBytes = recv(hSocket,message,BUFSIZE,0);
			if (receiveBytes > 0)
			{
				printf("TRACE - Receive message : %s (%d bytes)\n* Enter Message\n->", message, receiveBytes);
			}            
		}
		//Send(hSocket,dataBuf,message,overlapped);
		//Recv(hSocket,dataBuf,message,overlapped);
	}

	//** �޴� �κ� ����, �ִ� �κ� ����, �����尡 �ʿ��� ����

	WSACleanup();
	//WSACloseEvent(event);
	closesocket(hSocket);
	WSACleanup();
}

void Send(SOCKET &hSocket,WSABUF &dataBuf,char* message,WSAOVERLAPPED &overlapped)
{
	int sendBytes;
	// �Է� �޼����� ���ۿ� ����
	dataBuf.len = strlen(message);
	dataBuf.buf = message;

	if(WSASend(hSocket, &dataBuf, 1, (LPDWORD)&sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if(WSAGetLastError() == WSA_IO_PENDING)
		{
			//* ���� 2��, ��� ����ν�� ���� ������ - ���� ���� �ʿ�
			// WSA �̺�Ʈ�� ��ٸ���, ���ۿϷḦ ��ٸ���.
			WSAWaitForMultipleEvents( 1, &(overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );

			// ������� �޾ƿ´�.(��� 10014 ������ �߻��ϹǷ� ����Դµ�)
			WSAGetOverlappedResult( hSocket, &overlapped, (LPDWORD)&sendBytes, FALSE, NULL );
		}
		else
		{
			ErrorHandling("WSASend() error : %d",WSAGetLastError());
		}
	}
	printf("Send[%s]\n",dataBuf.buf);
}

void Recv(SOCKET &hSocket,WSABUF &dataBuf,char* message,WSAOVERLAPPED &overlapped)
{
	int recvBytes;
	int flags = 0;

	// ���� �ϳ����� ���� ������ ��� ������ ������ �޸� �Ʒ� ������ ���ʿ�
	/*
	// ���۸� ���� ���� �ʱ�ȭ
	memset(&(pSocketInfo->overlapped),0,sizeof(OVERLAPPED));
	pSocketInfo->wsaBuf.len = BUFSIZE;
	pSocketInfo->wsaBuf.buf = pSocketInfo->buffer;
	*/

	if(WSARecv(hSocket,&dataBuf,1,(LPDWORD)&recvBytes,(LPDWORD)&flags,&overlapped,NULL) == SOCKET_ERROR)
	{
		if(WSAGetLastError() == WSA_IO_PENDING)
		{
			//* ���� 2��, ��� ����ν�� ���� ������ - ���� ���� �ʿ�
			// WSA �̺�Ʈ�� ��ٸ���, ���ۿϷḦ ��ٸ���.
			WSAWaitForMultipleEvents( 1, &(overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );

			// ������� �޾ƿ´�.(��� 10014 ������ �߻��ϹǷ� ����Դµ�)
			WSAGetOverlappedResult( hSocket, &overlapped, (LPDWORD)&recvBytes, FALSE, NULL );
		}
		else
		{
			ErrorHandling("WSARecv() error : %d",WSAGetLastError());
		}
	}
	printf("Recv[%s]\n",dataBuf.buf);
}

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
