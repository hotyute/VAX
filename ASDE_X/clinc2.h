#ifndef CLINC2_H
#define CLINC2_H

#include <windows.h>
#include <winsock.h>
#include <iostream>
#include <string>
#include <tchar.h>

#include "Stream.h"


class tcpinterface {
public:
	SOCKET sConnect;
	tcpinterface();
	static DWORD WINAPI staticStart(void*);
	DWORD run();
	void sendMessage(Stream*);
	void startT(HWND);
	int connectNew(HWND, std::string, unsigned short);

	char message[5000];
	int packetSize;
	unsigned char packetType;
	Stream* out_stream;
	bool hand_shake;
	int current_op;
};

extern tcpinterface* intter;

#endif