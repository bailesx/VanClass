#include "IOCP_Server.h"

void main(int argc, char** argv)
{
	// Winsock Start - windock.dll 로드
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		ErrorHandling("ERROR - Failed WSAStartup() : %d", WSAGetLastError());
	}

	// IOCP의 소켓은 반드시 중첩 입출력 방식(OVERLAPPED)으로 생성하여야 함
	SOCKET hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET)
	{
		WSACleanup();
		ErrorHandling("ERROR - Failed WSASocket() : %d", WSAGetLastError());
	}

	// 서버 정보 객체 설정
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

	// 완료결과를 처리하는 객체(CP : Completion Port) 생성
	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// 쓰레드를 생성할 때 CPU의 개수를 고려해야 하므로 시스템 정보를 획득
	// 필요한 부분은 CPU의 쓰레드 개수이며, 멤버변수 dwNumberOfProcessors에서 확인 가능
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	int threadCount = SystemInfo.dwNumberOfProcessors * 2;
	//** 이 쓰레드 ID의 용도를 확인하면 이용 가치가 있을 듯
	unsigned threadID;
	// 쓰레드 핸들러
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

		Log("클라이언트가 접속하였습니다 : %s\n",inet_ntoa(clntAddr.sin_addr));

		// CreateIoCompleationPort의 두 번째 기능으로, 현재 연결된 클라이언트(overlapped)와 hIOCP를 연결
		hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (DWORD)socketInfo, 0);

		// 중첩 소켓을 지정하고 완료시 실행될 함수를 넘겨준다.
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

	//** 모든 클라이언트 종료 함수 콜 추가부
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
//			//* 이하 2줄, 사실 현재로써는 거의 무쓸모 - 차후 연구 필요
//			// WSA 이벤트를 기다린다, 전송완료를 기다린다.
//			WSAWaitForMultipleEvents( 1, &(PerIoData->overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );
//
//			// 결과값을 받아온다.(사실 10014 에러가 발생하므로 못써먹는듯)
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
//	// 데이터를 받기 전에 버퍼를 비우고 설정 초기화
//	memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
//	PerIoData->wsaBuf.len = BUFSIZE;
//	PerIoData->wsaBuf.buf = PerIoData->buffer;
//
//	if(WSARecv(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, (LPDWORD)&recvBytes, (LPDWORD)&flags, &(PerIoData->overlapped), NULL)==SOCKET_ERROR)
//	{
//		if(WSAGetLastError() == WSA_IO_PENDING)
//		{
//			//* 이하 2줄, 사실 현재로써는 거의 무쓸모 - 차후 연구 필요
//			// WSA 이벤트를 기다린다, 전송완료를 기다린다.
//			WSAWaitForMultipleEvents( 1, &(PerIoData->overlapped.hEvent), TRUE, WSA_INFINITE, FALSE );
//
//			// 결과값을 받아온다.(사실 10014 에러가 발생하므로 못써먹는듯)
//			WSAGetOverlappedResult( PerHandleData->hClntSock, &(PerIoData->overlapped), (LPDWORD)&recvBytes, FALSE, NULL );
//		}
//		else
//		{
//			ErrorHandling("WSARecv() error : %d",WSAGetLastError());
//		}
//	}
//	printf_s("Recv[%s]\n",PerIoData->wsaBuf.buf);
//}

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