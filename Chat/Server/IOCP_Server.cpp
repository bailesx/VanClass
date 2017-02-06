#include "IOCP_Server.h"

void main(int argc, char** argv)
{
	WSADATA wsaData;
	// CompletionPort의 Handle
	HANDLE hCompletionPort;
	// 쓰레드를 생성할 때 CPU의 개수를 고려해야 하므로 시스템 정보를 획득
	SYSTEM_INFO SystemInfo;

	// 서버 소켓 정보
	SOCKET hServSock;
	SOCKADDR_IN servAddr;

	// 클라이언트 소켓 정보
	socketinfo* pSocketInfo;

	// Load Winsock 2.2 DLL
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		ErrorHandling("WSAStartup() error : %d",WSAGetLastError());
	}

	// 1.CompletionPort 생성
	// CreateCompletionPort의 첫 번째에 해당하며, CompletionPort 생성부
	// 은행을 비유로 들자면 은행 건물 건축이다
	hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);

	// 시스템 정보 획득
	// 필요한 부분은 CPU의 쓰레드 개수이며, 멤버변수 dwNumberOfProcessors에서 확인 가능
	GetSystemInfo(&SystemInfo);
	unsigned int iNumThreads = SystemInfo.dwNumberOfProcessors * 2;

	// 2.CompletionPort에서 입출력 완료를 기다리는 쓰레드를 CPU의 2배로 생성
	// 은행을 비유로 들자면 은행원을 고용한 셈이다
	for(unsigned int i = 0; i<iNumThreads; i++)
	{
		_beginthreadex(NULL,0,CompletionThread,(LPVOID)hCompletionPort,0,NULL);
	}

	// IOCP의 소켓은 반드시 중첩 입출력 방식(OVERLAPPED)으로 생성하여야 함
	hServSock = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port=htons(atoi("6664"));

	bind(hServSock,(SOCKADDR*)&servAddr,sizeof(servAddr));
	listen(hServSock,5);

	while(true)
	{
		printf("//**메인함수반복문 시작\n");
		SOCKET hSocket;
		SOCKADDR_IN compAddr;
		int addrLen = sizeof(compAddr);

		hSocket = accept(hServSock,(SOCKADDR*)&compAddr,&addrLen);
		Log("클라이언트가 접속하였습니다 : %s\n",inet_ntoa(compAddr.sin_addr));
		
		// 연결된 클라이언트의 소켓 정보를 동적할당으로 설정
		pSocketInfo = new SocketInfo;
		pSocketInfo->hSocket = hSocket;
		memcpy(&(pSocketInfo->compAddr),&compAddr,addrLen);

		// 3.Overlapped 소켓과 CompletionPort의 연결
		// CreateIoCompleationPort의 두 번째 기능으로, 현재 연결된 클라이언트와 hCompletionPort를 연결
		// 세 번째 인자로 클라이언트 정보를 담았던 pSocketInfo를 전달
		// 은행을 비유로 들자면 손님의 입장이 되시겠다
		CreateIoCompletionPort((HANDLE)pSocketInfo->hSocket,hCompletionPort,(DWORD)pSocketInfo,0);
		
		// 4.중첩된 데이터 입력?
		Recv(pSocketInfo);
		printf("//**메인함수반복문 리시브\n");
	}

	//** 모든 클라이언트 종료 함수 콜 추가부
	closesocket(hServSock);
	WSACleanup();
}

// 입출력 완료에 따른 쓰레드의 행동 정의이며, 실제 기능 구현부... 아마...? switch문을 이용하면...
unsigned int __stdcall CompletionThread(HANDLE hCompletionPort)
{
	DWORD BytesTransferred;
	socketinfo* pSocketInfo;

	while(true)
	{
		printf("//**쓰레드함수반복문 시작\n");
		// 5. 입출력이 완료된 소켓 정보 획득
		// 첫번째 인자로 전달된 hCompletionPort에 연결된 소켓들이 완료될 때까지 대기
		// 이후, 완료되는 소켓부터 리턴
		GetQueuedCompletionStatus(hCompletionPort,
			&BytesTransferred,						// 전송된 바이트 수
			(LPDWORD)&pSocketInfo,
			(LPOVERLAPPED*)&pSocketInfo,				// overlapped 구조체 포인터
			INFINITE);
		// 세 번째 인자로 전달된 pSocketInfo는 입출력이 완료된 소켓으로,
		// hCompletionPort와 연결할 때 인자로 전달한 클라이언트 정보가 전달됨

		// 네 번째 인자로 전달된 pSocketInfo는 WSA Send/Recv 시에 전달한
		// overlapped 멤버 변수의 포인터를 얻기 위해 사용

		// (datainfo 구조체의 첫 번째 멤버 변수는 overlapped
		// 따라서 둘의 시작 주소는 동일하기에 WSARecv에서 overlapped의 주소를 넘겨도
		// 이곳에서 LPPER_ID_DATA로 사용 가능)

		// EOF 발생
		if(!BytesTransferred)
		{
			// 클라이언트 연결 종료 시 처리
			closesocket(pSocketInfo->hSocket);
			delete pSocketInfo;
			delete pSocketInfo;

			continue;
		}
		
		// 버퍼의 끝에 문자열 종료 메세지 추가
		pSocketInfo->wsaBuf.buf[BytesTransferred] = '\0';
		printf("Recv[%s]\n",pSocketInfo->wsaBuf.buf);

		// 6. 클라이언트로 메세지 에코
		pSocketInfo->wsaBuf.len = BytesTransferred;
		Send(pSocketInfo);
		printf("//**쓰레드함수반복문 샌드\n");
		
		Recv(pSocketInfo);
		printf("//**쓰레드함수반복문 리시브\n");
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
	
	// 버퍼를 비우고 설정 초기화
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