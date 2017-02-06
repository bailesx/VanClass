#include "IOCP_Client.h"

void main()
{
	WSADATA wsaData;
	socketinfo* pSocketInfo = new SocketInfo;

	char strIp[16] = {0,};
	//**char strNickname[21] = {0,};
	
	// WSA ����, ���� ���� 0�̶�� ����
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		ErrorHandling("WSAStartup() error : %d",WSAGetLastError());
	}

	pSocketInfo->hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(pSocketInfo->hSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error : %d",WSAGetLastError());
	}

	// ip �Է�
	printf("������ ������ IP�� �Է��ϼ��� : ");
	scanf_s("%s",strIp,sizeof(strIp));
	fflush(stdin);

	memset(&pSocketInfo->compAddr,0,sizeof(pSocketInfo->compAddr));
	pSocketInfo->compAddr.sin_family = AF_INET;
	pSocketInfo->compAddr.sin_addr.s_addr = inet_addr(strIp);
	pSocketInfo->compAddr.sin_port = htons(6664);

	if(connect(pSocketInfo->hSocket,(SOCKADDR*)&pSocketInfo->compAddr,sizeof(pSocketInfo->compAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error : %d",WSAGetLastError());
	}

	// ����ü�� �̺�Ʈ �ڵ� �����ؼ� ����
	WSAEVENT event = WSACreateEvent();
	memset(&(pSocketInfo->overlapped),0,sizeof(pSocketInfo->overlapped));
	pSocketInfo->overlapped.hEvent = event;

	printf("���ӿ� �����Ͽ����ϴ�!\n");
	//**printf("�г����� �Է����ּ���(�ѱ�10��/����20�� �̳�) : ");
	//scanf_s("%s",strNickname,sizeof(strNickname));
	//fflush(stdin);
	
	printf("ä�� �濡 ���� ���� ȯ���մϴ�!\n");
	printf("������ -help�Դϴ�.\n");
	printf("�޼����� �Է��ϼ���.\n");

	// ������ ������
	while(true)
	{
		scanf_s("%s",pSocketInfo->buffer,sizeof(pSocketInfo->buffer));
		fflush(stdin);

		// �Է� �޼����� exit��� ����
		if(!strcmp(pSocketInfo->buffer,"exit")) break;

		Send(pSocketInfo);
		// ���� �Ϸ� Ȯ��
		if(WSAWaitForMultipleEvents(1,&event,TRUE,WSA_INFINITE,FALSE) != WSA_WAIT_EVENT_0)
		{
			ErrorHandling("WSAWaitForMultipleEvents() error : %d",WSAGetLastError());
		}
				
		//** ���۵� ����Ʈ �� Ȯ��
		// 10014 ������ �߻��ؼ� ����԰���
		//if( WSAGetOverlappedResult(pSocketInfo->hSocket,&(pSocketInfo->overlapped),(LPDWORD)pSocketInfo->wsaBuf.len,FALSE,NULL) != TRUE)
		//{
		//	ErrorHandling("WSAGetOverlappedResult() error : %d",WSAGetLastError());
		//}
		//printf("���۵� ����Ʈ �� : %d\n",pSocketInfo->wsaBuf.len);

		Recv(pSocketInfo);
	}
	//** �޴� �κ� ����, �ִ� �κ� ����, �����尡 �ʿ��� ����

	WSACloseEvent(event);
	closesocket(pSocketInfo->hSocket);
	WSACleanup();
}

void Send(socketinfo* pSocketInfo)
{
	// �Է� �޼����� ���ۿ� ����
	pSocketInfo->wsaBuf.buf = pSocketInfo->buffer;
	pSocketInfo->wsaBuf.len = strlen(pSocketInfo->buffer);

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

	/* Ŭ��� �ʿ���� ��?
	// ���۸� ���� ���� �ʱ�ȭ
	memset(&(pSocketInfo->overlapped),0,sizeof(OVERLAPPED));
	pSocketInfo->wsaBuf.len = BUFSIZE;
	pSocketInfo->wsaBuf.buf = pSocketInfo->buffer;
	*/
	//** �����κ��� ����� ���ڸ� �ް� �Ǹ� �ʿ����������
	memset(pSocketInfo->wsaBuf.buf,0,BUFSIZE);

	if(WSARecv(pSocketInfo->hSocket,&(pSocketInfo->wsaBuf),1,(LPDWORD)&pSocketInfo->recvbytes,(LPDWORD)&flags,&(pSocketInfo->overlapped),CompRoutine)==SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			ErrorHandling("WSARecv() error : %d",WSAGetLastError());
		}
	}
	printf("Recv[%s]\n",pSocketInfo->wsaBuf.buf);
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
