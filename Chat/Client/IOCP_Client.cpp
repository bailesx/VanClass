#include "IOCP_Client.h"

void main()
{
	WSADATA wsaData;
	socketinfo* pSocketInfo = new SocketInfo;

	char strIp[16] = {0,};
	//**char strNickname[21] = {0,};
	
	// WSA 시작, 리턴 값이 0이라면 성공
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		ErrorHandling("WSAStartup() error : %d",WSAGetLastError());
	}

	pSocketInfo->hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(pSocketInfo->hSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error : %d",WSAGetLastError());
	}

	// ip 입력
	printf("접속할 서버의 IP를 입력하세요 : ");
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

	// 구조체에 이벤트 핸들 삽입해서 전달
	WSAEVENT event = WSACreateEvent();
	memset(&(pSocketInfo->overlapped),0,sizeof(pSocketInfo->overlapped));
	pSocketInfo->overlapped.hEvent = event;

	printf("접속에 성공하였습니다!\n");
	//**printf("닉네임을 입력해주세요(한글10자/영문20자 이내) : ");
	//scanf_s("%s",strNickname,sizeof(strNickname));
	//fflush(stdin);
	
	printf("채팅 방에 오신 것을 환영합니다!\n");
	printf("도움말은 -help입니다.\n");
	printf("메세지를 입력하세요.\n");

	// 전송할 데이터
	while(true)
	{
		scanf_s("%s",pSocketInfo->buffer,sizeof(pSocketInfo->buffer));
		fflush(stdin);

		// 입력 메세지가 exit라면 종료
		if(!strcmp(pSocketInfo->buffer,"exit")) break;

		Send(pSocketInfo);
		// 전송 완료 확인
		if(WSAWaitForMultipleEvents(1,&event,TRUE,WSA_INFINITE,FALSE) != WSA_WAIT_EVENT_0)
		{
			ErrorHandling("WSAWaitForMultipleEvents() error : %d",WSAGetLastError());
		}
				
		//** 전송된 바이트 수 확인
		// 10014 에러가 발생해서 못써먹겠음
		//if( WSAGetOverlappedResult(pSocketInfo->hSocket,&(pSocketInfo->overlapped),(LPDWORD)pSocketInfo->wsaBuf.len,FALSE,NULL) != TRUE)
		//{
		//	ErrorHandling("WSAGetOverlappedResult() error : %d",WSAGetLastError());
		//}
		//printf("전송된 바이트 수 : %d\n",pSocketInfo->wsaBuf.len);

		Recv(pSocketInfo);
	}
	//** 받는 부분 따로, 주는 부분 따로, 쓰레드가 필요한 시점

	WSACloseEvent(event);
	closesocket(pSocketInfo->hSocket);
	WSACleanup();
}

void Send(socketinfo* pSocketInfo)
{
	// 입력 메세지를 버퍼에 담음
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

	/* 클라는 필요없는 듯?
	// 버퍼를 비우고 설정 초기화
	memset(&(pSocketInfo->overlapped),0,sizeof(OVERLAPPED));
	pSocketInfo->wsaBuf.len = BUFSIZE;
	pSocketInfo->wsaBuf.buf = pSocketInfo->buffer;
	*/
	//** 서버로부터 제대로 에코를 받게 되면 필요없어질지도
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
