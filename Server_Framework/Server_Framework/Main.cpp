


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <process.h>

#include "main.h"

/////////////////////////////////////////////////////////////////////
//서버 관련 전역 변수
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// 서버초기화.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////
BOOL InitServer(void)
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// 서버삭제.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////
BOOL ReleaseServer(void)
{

	return TRUE;
}



/////////////////////////////////////////////////////////////////////
// 메인 함수.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////
void main(void)
{

	//---------------------------------------------------------------
	// 윈속 스타트 업.
	//---------------------------------------------------------------
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	HANDLE hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	//---------------------------------------------------------------
	// 스레드 풀 생성
	//---------------------------------------------------------------
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	for (int i = 0; i<SystemInfo.dwNumberOfProcessors; ++i)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)hCompletionPort, 0, NULL);
	//---------------------------------------------------------------
	// 소켓 생성, 주소 등록
	//---------------------------------------------------------------
	SOCKET hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(2738);
	//---------------------------------------------------------------
	// 서버 포트 바인딩 및 리슨 대기
	//---------------------------------------------------------------
	bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr));
	listen(hServSock, 5);
	cout << "#Finish Bind" << endl << "#Port Number 2738" << endl
		<< "#Waiting for Accept" << endl;
	//---------------------------------------------------------------
	// 네트워크 처리.
	//---------------------------------------------------------------
		
	LPPER_IO_DATA PerIoData;
	LPPER_HANDLE_DATA PerHandleData;

	int RecvBytes;
	int i, Flags;
	
	while (TRUE)
	{
		
		if (SessionID < 5) {
			SOCKADDR_IN clntAddr;
			int addrLen = sizeof(clntAddr);
			SOCKET hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);
			cout << "#Accept완료" << endl << "연결된 ip주소:" << inet_ntoa(clntAddr.sin_addr) << endl;

			PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
			PerHandleData->hClntSock = hClntSock;
			memcpy(&(PerHandleData->clntAddr), &clntAddr, addrLen);

			CreateIoCompletionPort((HANDLE)hClntSock, hCompletionPort, (DWORD)PerHandleData, 0);

			PerIoData = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
			PerIoData->wsaBuf.len = BUFSIZE;
			PerIoData->wsaBuf.buf = PerIoData->buffer;
			Flags = 0;

			WSARecv(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1,
				(LPDWORD)&RecvBytes, (LPDWORD)&Flags, &(PerIoData->overlapped), NULL);
		}
	}
	




}
/////////////////////////////////////////////////////////////////////
// 스레드 풀 생성 함수
//
// Parameters: message내용.
// Return: 없음.
/////////////////////////////////////////////////////////////////////
unsigned int __stdcall CompletionThread(LPVOID pComPort)
{
	HANDLE hCompletionPort = (HANDLE)pComPort;
	DWORD BytesTransferred;
	LPPER_HANDLE_DATA PerHandleData;

	DWORD flags;

	//시간 나타낼 변수
	struct tm *t;
	time_t timer;
	LPPER_IO_DATA PerIoData;


	while (1) {

		GetQueuedCompletionStatus(hCompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&PerIoData, INFINITE);

		if (BytesTransferred == 0)
		{
			closesocket(PerHandleData->hClntSock);
			free(PerHandleData);
			free(PerIoData);
			continue;
		}

		PerIoData->wsaBuf.buf[BytesTransferred] = '\0';
		timer = time(NULL);    // 현재 시각을 초 단위로 얻기
		t = localtime(&timer); // 초 단위의 시간을 분리하여 구조체에 넣기
		printf("%s   ", timeToString(t));
		printf("%s\n", PerIoData->wsaBuf.buf);

		PerIoData->wsaBuf.len = BytesTransferred;
		memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
		WSASend(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, NULL, 0, NULL, NULL);


		PerIoData->wsaBuf.len = BUFSIZE;
		PerIoData->wsaBuf.buf = PerIoData->buffer;

		flags = 0;

		WSARecv(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, NULL, &flags, &(PerIoData->overlapped), NULL);
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////
// 시간 출력 함수.
//
// Parameters: time 구조체.
// Return: 문자열.
/////////////////////////////////////////////////////////////////////
char* timeToString(struct tm *t) {
	static char s[20];

	sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec
	);

	return s;
}

/////////////////////////////////////////////////////////////////////
// 에러 출력 함수.
//
// Parameters: message내용.
// Return: 없음.
/////////////////////////////////////////////////////////////////////
void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
