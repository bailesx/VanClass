#include "IOCP_Client.h"

void main()
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN recvAddr;
	char ip[16] = {0,};

	// ������ ���� ����ü
	WSABUF dataBuf;
	char message[1024] = {0,};
	int sendBytes = 0;

	// �̺�Ʈ �ڵ�
	WSAEVENT event;
	WSAOVERLAPPED overlapped;

	// WSA ����, ���� ���� 0�̶�� ����
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		ErrorHandling("WSAStartup() error : %d",WSAGetLastError());
	}

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(hSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error : %d",WSAGetLastError());
	}

	// ip �Է�
	printf("������ ������ IP�� �Է��ϼ��� : ");
	scanf_s("%s",ip,sizeof(ip));

	memset(&recvAddr,0,sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr(ip);
	recvAddr.sin_port = htons(6664);

	if(connect(hSocket,(SOCKADDR*)&recvAddr,sizeof(recvAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error : %d",WSAGetLastError());
	}

	// ����ü�� �̺�Ʈ �ڵ� �����ؼ� ����
	event = WSACreateEvent();
	memset(&overlapped,0,sizeof(overlapped));
	overlapped.hEvent = event;

	printf("ä�� �濡 ���� ���� ȯ���մϴ�!\n");
	printf("������ -help�Դϴ�.\n");
	printf("�޼����� �Է��ϼ���.\n");
	// ������ ������
	while(true)
	{
		scanf_s("%s",message,sizeof(message));

		// �Է� �޼����� exit��� ����
		if(!strcmp(message,"exit")) break;

		// �Է� �޼����� ���ۿ� ����
		dataBuf.len = strlen(message);
		dataBuf.buf = message;

		if(WSASend(hSocket
			,&dataBuf
			,1
			,(LPDWORD)&sendBytes
			,0
			,&overlapped
			,NULL))
		{
			ErrorHandling("WSASend() error : %d",WSAGetLastError());
		}
		printf("%s\n",dataBuf.buf);
	}

	closesocket(hSocket);
	WSACleanup();
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