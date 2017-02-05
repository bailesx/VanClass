#include "IOCP_Server.h"

int main(int argc, char** argv)
{
	WSADATA wsaData;
	// 생성된 CompletionPort가 전달 될 Handle
	HANDLE hCompletionPort;

	// 쓰레드를 생성할 때 CPU의 개수를 고려 해야하므로 시스템 정보를 획득
	SYSTEM_INFO SystemInfo;

	SOCKADDR_IN servAddr;

	// 소켓의 버퍼 정보
	LPPER_IO_DATA PerIoData;

	// 소켓 정보
	LPPER_HANDLE_DATA PerHandleData;

	SOCKET hServSock;
	int RecvBytes;
	int Flags;

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

	// 2.CompletionPort에서 입출력 완료를 기다리는 쓰레드를 CPU 개수만큼 생성
	// CPU 개수만큼 쓰레드를 생성해주는 부분
	// 은행을 비유로 들자면 은행원을 고용한 셈이다
	for(unsigned int i = 0; i<SystemInfo.dwNumberOfProcessors; i++)
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
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		hClntSock = accept(hServSock,(SOCKADDR*)&clntAddr,&addrLen);
		Log("클라이언트가 접속하였습니다 : %s\n",inet_ntoa(clntAddr.sin_addr));
		
		// 연결된 클라이언트의 소켓 정보를 동적할당으로 설정
		PerHandleData = new PER_HANDLE_DATA;
		PerHandleData->hClntSock = hClntSock;
		memcpy(&(PerHandleData->clntAddr),&clntAddr,addrLen);

		// 3.Overlapped 소켓과 CompletionPort의 연결
		// CreateIoCompleationPort의 두 번째 기능으로, 현재 연결된 클라이언트와 hCompletionPort를 연결
		// 세 번째 인자로 클라이언트 정보를 담았던 PerHandleData를 전달
		// 은행을 비유로 들자면 손님의 입장이 되시겠다
		CreateIoCompletionPort((HANDLE)hClntSock,hCompletionPort,(DWORD)PerHandleData,0);

		// 연결된 클라이언트를 위한 버퍼를 설정하고 overlapped 구조체 변수 초기화
		// 소켓 버퍼 정보를 지닐 PerIoData 생성 및 초기화
		PerIoData = new PER_IO_DATA;
		memset(&(PerIoData->overlapped),0,sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;

		// 4.중첩된 데이터 입력
		WSARecv(PerHandleData->hClntSock,	// 데이터 입력 소켓
			&(PerIoData->wsaBuf),			// 데이터 입력 버퍼 포인터
			1,								// 데이터 입력 버퍼의 수
			(LPDWORD)&RecvBytes,
			(LPDWORD)&Flags,
			&(PerIoData->overlapped),		// overlapped 구조체 포인터
			NULL);
	}
	return 0;
}

// 입출력 완료에 따른 쓰레드의 행동 정의이며, 실제 기능 구현부
unsigned int __stdcall CompletionThread(LPVOID pComPort)
{
	//** 왜 굳이 인자 값에 LPVOID로 형변환의 필요성이 있는 것인가? 그냥 HANDLE 사용하면 안되나?
	HANDLE hCompletionPort = (HANDLE)pComPort;

	DWORD BytesTransferred;
	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_DATA PerIoData;
	DWORD flags;

	while(true)
	{
		// 5. 입출력이 완료된 소켓 정보 획득
		// 첫번째 인자로 전달된 hCompletionPort에 연결된 소켓들이 완료될 때까지 대기
		// 이후, 완료되는 소켓부터 리턴
		GetQueuedCompletionStatus(hCompletionPort,
			&BytesTransferred,						// 전송된 바이트 수
			(LPDWORD)&PerHandleData,
			(LPOVERLAPPED*)&PerIoData,				// overlapped 구조체 포인터
			INFINITE);
		// 세 번째 인자로 전달된 PerHandleData는 입출력이 완료된 소켓으로,
		// hCompletionPort와 연결할 때 인자로 전달한 클라이언트 정보가 전달됨

		// 네 번째 인자로 전달된 PerIoData는 WSA Send/Recv 시에 전달한
		// overlapped 멤버 변수의 포인터를 얻기 위해 사용

		// (LPPER_IO_DATA 구조체의 첫 번째 멤버 변수는 overlapped
		// 따라서 둘의 시작 주소는 동일하기에 WSARecv에서 overlapped의 주소를 넘겨도
		// 이곳에서 LPPER_ID_DATA로 사용 가능)

		// EOF 발생
		if(!BytesTransferred)
		{
			// 클라이언트 연결 종료 시 처리
			closesocket(PerHandleData->hClntSock);
			delete PerHandleData;
			delete PerIoData;

			continue;
		}
		
		// 버퍼의 끝에 문자열 종료 메세지 추가
		PerIoData->wsaBuf.buf[BytesTransferred] = '\0';
		printf("Recv[%s]\n",PerIoData->wsaBuf.buf);

		// 6. 클라이언트로 메세지 에코
		PerIoData->wsaBuf.len = BytesTransferred;
		WSASend(PerHandleData->hClntSock,&(PerIoData->wsaBuf),1,NULL,0,NULL,NULL);

		// 버퍼를 비우고 설정 초기화
		memset(&(PerIoData->overlapped),0,sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;

		flags = 0;
		WSARecv(PerHandleData->hClntSock,
			&(PerIoData->wsaBuf),
			1,
			NULL,
			&flags,
			&(PerIoData->overlapped),
			NULL);
	}
	return 0;
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