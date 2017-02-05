#include "IOCP_Client.h"

void main()
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN recvAddr;
	char ip[16] = {0,};

	// 데이터 버퍼 구조체
	WSABUF dataBuf;
	char message[1024] = {0,};
	int sendBytes = 0;

	// 이벤트 핸들
	WSAEVENT event;
	WSAOVERLAPPED overlapped;

	// WSA 시작, 리턴 값이 0이라면 성공
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		ErrorHandling("WSAStartup() error : %d",WSAGetLastError());
	}

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(hSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error : %d",WSAGetLastError());
	}

	// ip 입력
	printf("접속할 서버의 IP를 입력하세요 : ");
	scanf_s("%s",ip,sizeof(ip));

	memset(&recvAddr,0,sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr(ip);
	recvAddr.sin_port = htons(6664);

	if(connect(hSocket,(SOCKADDR*)&recvAddr,sizeof(recvAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error : %d",WSAGetLastError());
	}

	// 구조체에 이벤트 핸들 삽입해서 전달
	event = WSACreateEvent();
	memset(&overlapped,0,sizeof(overlapped));
	overlapped.hEvent = event;

	printf("채팅 방에 오신 것을 환영합니다!\n");
	printf("도움말은 -help입니다.\n");
	printf("메세지를 입력하세요.\n");
	// 전송할 데이터
	while(true)
	{
		scanf_s("%s",message,sizeof(message));

		// 입력 메세지가 exit라면 종료
		if(!strcmp(message,"exit")) break;

		// 입력 메세지를 버퍼에 담음
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