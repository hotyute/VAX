#pragma once


#include <sdkddkver.h>

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <string>

class tcpinterface1 {
public:
	SOCKET sConnect;
	tcpinterface1();
	static DWORD WINAPI staticStart(void*);
	DWORD run();
	DWORD sendMessage(std::string);
	void startT(HWND);
	int connectNew(HWND, std::string, unsigned short);
};