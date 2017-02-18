#include "IOCP_Client.h"

void main()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		ErrorHandling("ERROR - Failed WSAStartup() : %d",WSAGetLastError());
	}

	// 소켓 생성
	SOCKET hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(hSocket == INVALID_SOCKET)
	{
		WSACleanup();
		ErrorHandling("ERROR - Failed WSASocket() : %d",WSAGetLastError());
	}

	// 서버 정보 객체 설정
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
	// 구조체에 이벤트 핸들 삽입해서 전달
	WSAEVENT event = WSACreateEvent();
	WSAOVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = event;
	*/

	printf("접속에 성공하였습니다!\n");
	printf("채팅 방에 오신 것을 환영합니다!\n");
	printf("도움말은 -help입니다.\n");
	printf("메세지를 입력하세요.\n");

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

		// 입력 메세지가 exit라면 종료
		if(!strcmp(message,"exit")) break;

		/* 클라이언트에서 IOCP 내용을 쓰려면 아래 구문 참조(서버와 유사)
		socketInfo = (struct SOCKETINFO *)malloc(sizeof(struct SOCKETINFO));
		memset((void *)socketInfo, 0x00, sizeof(struct SOCKETINFO));        
		socketInfo->dataBuffer.len = bufferLen;
		socketInfo->dataBuffer.buf = messageBuffer;
		*/

		// 3-1. 데이터 쓰기
		int sendBytes = send(hSocket,message,bufferLen,0);
		if (sendBytes > 0)
		{
			printf("TRACE - Send message : %s (%d bytes)\n",message,sendBytes);
			// 3-2. 데이터 읽기
			int receiveBytes = recv(hSocket,message,BUFSIZE,0);
			if (receiveBytes > 0)
			{
				printf("TRACE - Receive message : %s (%d bytes)\n* Enter Message\n->", message, receiveBytes);
			}            
		}
		//Send(hSocket,dataBuf,message,overlapped);
		//Recv(hSocket,dataBuf,message,overlapped);
	}

	//** 받는 부분 따로, 주는 부분 따로, 쓰레드가 필요한 시점

	WSACleanup();
	//WSACloseEvent(event);
	closesocket(hSocket);
	WSACleanup();
}

void Send(SOCKET &hSocket,WSABUF &dataBuf,char* message,WSAOVERLAPPED &overlapped)
{
	int sendBytes;
	// 입력 메세지를 버퍼에 담음
	dataBuf.len = strlen(message);
	dataBuf.buf = message;

	if(WSASend(hSocket, &dataBuf, 1, (LPDWORD)&sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if(WSAGetLastError() == WSA_IO_PENDING)
		{
			//* 이하 2줄, 사실 현재로써는 거의 무쓸모 - 차후 연구 필요
			// WSA 이벤트를 기다린다, 전송완료를 기다린다.
			WSAWaitForMultipleEvents( 1, &(overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );

			// 결과값을 받아온다.(사실 10014 에러가 발생하므로 못써먹는듯)
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

	// 서버 하나만을 통해 정보를 얻기 때문에 서버와 달리 아래 구문은 불필요
	/*
	// 버퍼를 비우고 설정 초기화
	memset(&(pSocketInfo->overlapped),0,sizeof(OVERLAPPED));
	pSocketInfo->wsaBuf.len = BUFSIZE;
	pSocketInfo->wsaBuf.buf = pSocketInfo->buffer;
	*/

	if(WSARecv(hSocket,&dataBuf,1,(LPDWORD)&recvBytes,(LPDWORD)&flags,&overlapped,NULL) == SOCKET_ERROR)
	{
		if(WSAGetLastError() == WSA_IO_PENDING)
		{
			//* 이하 2줄, 사실 현재로써는 거의 무쓸모 - 차후 연구 필요
			// WSA 이벤트를 기다린다, 전송완료를 기다린다.
			WSAWaitForMultipleEvents( 1, &(overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );

			// 결과값을 받아온다.(사실 10014 에러가 발생하므로 못써먹는듯)
			WSAGetOverlappedResult( hSocket, &overlapped, (LPDWORD)&recvBytes, FALSE, NULL );
		}
		else
		{
			ErrorHandling("WSARecv() error : %d",WSAGetLastError());
		}
	}
	printf("Recv[%s]\n",dataBuf.buf);
}

//* 함수만 만들었지, 무쓸모
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
