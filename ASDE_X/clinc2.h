#pragma once

#define _WINSOCK2API_
#include <mutex>
#include <string>
#include <windows.h>
#include <WinSock2.h>

#include "basic_stream.h"
#include "events.h"


class tcpinterface {
public:
	std::mutex writeMutex;
	SOCKET sConnect;
	tcpinterface();
	static DWORD WINAPI staticStart(void*);
	DWORD run();
	void sendMessage(BasicStream* stream);
	void send_data(SOCKET clientSocket, const std::vector<char>& buffer);
	void startT(HWND);
	int disconnect_socket();
	int connectNew(HWND, std::string, unsigned short);
	Event* position_updates = nullptr;

	BasicStream* in_stream;
	bool hand_shake;
	int current_op = -1;

	int nBytesReceived = 0;
	TIMEVAL timeout1;
	int TimeoutSec1 = 15; //
	fd_set rfds;
	int retval;
	bool closed = true;
};

extern tcpinterface* intter;

void decode(BasicStream& in);
