#ifndef CLINC2_H
#define CLINC2_H

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <tchar.h>

#include "Stream.h"


class tcpinterface {
private:
	HANDLE writeMutex;
public:
	SOCKET sConnect;
	tcpinterface();
	static DWORD WINAPI staticStart(void*);
	DWORD run();
	void sendMessage(Stream*);
	void startT(HWND);
	int connectNew(HWND, std::string, unsigned short);

	char message[5000];
	Stream* in_stream;
	bool hand_shake;
	int current_op = -1;

	int nBytesReceived = 0;
	TIMEVAL timeout1;
	int TimeoutSec1 = 30; //
	fd_set rfds;
	int retval;

	void w_lock();
	void w_unlock();
};

extern tcpinterface* intter;

#endif

void decode(Stream& in);
