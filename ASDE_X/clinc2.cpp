#include "clinc2.h"

#include <thread>
#include <iostream>
#include <chrono>

#include "config.h"
#include "usermanager.h"

using namespace std::chrono_literals;

tcpinterface* intter = new tcpinterface();

static int packetSizes[256][2] = 
{
	{10, 8},
	{13, 0},
	{9, -2},
	{14, 36},
	{12, 2},
	{15, -2},
	{16, 3},
	{11, -2},
	{17, -2},
	{18, 19},
	{19, 4}
};

tcpinterface::tcpinterface() {
	this->in_stream = new Stream(5000);
	this->in_stream->clearBuf();
	memset(tcpinterface::message, 0, 5000);
	timeout1.tv_sec = TimeoutSec1;
	timeout1.tv_usec = 0;
	sConnect = INVALID_SOCKET;
}

DWORD WINAPI tcpinterface::staticStart(void* param) {
	tcpinterface* tcp = (tcpinterface*)param;
	return tcp->run();
}

DWORD tcpinterface::run() {
	while (!quit) {
		if (closed)
		{
			auto start = std::chrono::high_resolution_clock::now();
			std::this_thread::sleep_for(30ms);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;
			continue;
		}

		ZeroMemory(message, sizeof(message));

		FD_ZERO(&rfds);
		FD_SET(tcpinterface::sConnect, &rfds);

		retval = select(tcpinterface::sConnect + 1, &rfds, 0, 0, &timeout1);

		if (FD_ISSET(tcpinterface::sConnect, &rfds))
		{
			nBytesReceived = recv(tcpinterface::sConnect, message, 5000, 0);
			if (queue_clean)
			{
				in_stream->clearBuf();
				memset(tcpinterface::message, 0, 5000);
				//disconnect();
				closed = true;
				printf("Connection was closed by remote person or timeout exceeded 60 seconds\n");
				queue_clean = false;
				break;
			}

			if (nBytesReceived == SOCKET_ERROR)
				break;

			if (nBytesReceived == 0)
				continue;

			Stream& in = *in_stream;
			memcpy(in.buffer + in.length, message, nBytesReceived);
			in.length += nBytesReceived;

			if (tcpinterface::hand_shake)
			{
				if (tcpinterface::current_op == 45)
				{
					in.markReaderIndex();
					if (nBytesReceived >= 11)
					{
						int loginCode = in.readUnsignedByte();
						int index = in.readUnsignedWord();
						long long updateTimeInMillis = in.readQWord();
						if (loginCode == 1)
						{
							//setIndex
							//setUpdateTimeinMillis
							//sendUpdates
							USER->setUserIndex(index);
							USER->setUpdateTime(updateTimeInMillis);
							this->position_updates = new PositionUpdates();
							position_updates->eAction.setTicks(0);
							event_manager1->addEvent(position_updates);
							tcpinterface::hand_shake = false;
							in.deleteReaderBlock();
						}
					}
					else
					{
						in.resetReaderIndex();
					}
				}
			}

			if (!hand_shake) {
				if (in.remaining() > 0)
				{
					decode(in);
				}
			}
		}
		FD_ZERO(&rfds);
		FD_SET(tcpinterface::sConnect, &rfds);

		retval = select(tcpinterface::sConnect + 1, &rfds, 0, 0, &timeout1);
	}
	if (retval == SOCKET_ERROR)
	{
		in_stream->clearBuf();
		disconnect();
		//do somethin
	}
	return 0;
}

void decode(Stream& in)
{
	while (in.remaining() > 0)
	{
		in.markReaderIndex();
		int opCode = in.readUnsignedByte(), length = -3;
		if (opCode != -1)
		{
			for (int j = 0; j < 256; j++)
			{
				if (packetSizes[j][0] == opCode)
				{
					length = packetSizes[j][1];
					break;
				}
			}
			if (length == -1)
			{
				if (in.remaining() >= 1)
				{
					length = in.readUnsignedByte();
				}
				else
				{
					in.resetReaderIndex();
					break;
				}
			}
			else if (length == -2)
			{
				if (in.remaining() >= 2)
				{
					length = in.readUnsignedWord();
				}
				else
				{
					in.resetReaderIndex();
					break;
				}
			}
			else if (length == -3)
			{
			#ifdef _DEBUG
				std::cout << "Unhandled Packet_Id!! : [" << (int)opCode << ", Packet_Size: "
					<< length << ", Bytes_Ava: " << in.remaining() << "] ... Skipping" << std::endl;
			#endif
				length = in.remaining();
			}
		#ifdef _DEBUG
			std::cout << "Packet_Id: " << (int)opCode << ", Packet_Size: " << length << ", Bytes_Ava: " << in.remaining() << std::endl;
		#endif
			if (in.remaining() >= length)
			{
				//handle
				decodePackets(opCode, in);
				in.deleteReaderBlock();
			}
			else
			{
				in.resetReaderIndex();
				break;
			}
		}
	}
}

void tcpinterface::sendMessage(Stream* stream) {
	if (stream->currentOffset == 0) {
		printf("Can't flush empty stream o.O\n");
		return;
	}

	w_lock();
	DWORD what = send(tcpinterface::sConnect, stream->buffer, stream->currentOffset, NULL);
	stream->currentOffset = 0;
	w_unlock();
}

void tcpinterface::startT(HWND hWnd) {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)staticStart, (void*)this, 0, NULL);
}

int tcpinterface::disconnect_socket() {
	int iResult = shutdown(tcpinterface::sConnect, 0x01);
	closed = true;
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(tcpinterface::sConnect);
		WSACleanup();
		return 1;
	}
	return 0;
}

int tcpinterface::connectNew(HWND hWnd, std::string saddr, unsigned short port) {
	int err;
	WSADATA wsaData;
	WORD DLLVersion;
	DLLVersion = MAKEWORD(2, 1);
	err = WSAStartup(DLLVersion, &wsaData);

	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return 0;
	}

	SOCKADDR_IN addr;

	int addrlen = sizeof(addr);

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	addr.sin_addr.s_addr = inet_addr(saddr.c_str());

	addr.sin_port = htons(port);

	addr.sin_family = AF_INET;

	int iResult = connect(tcpinterface::sConnect, (SOCKADDR*)&addr, sizeof(addr));

	if (iResult == SOCKET_ERROR) {
		int iError = WSAGetLastError();
		if (iError == WSAEWOULDBLOCK)
		{
		#ifdef _DEBUG
			std::cout << "Attempting to connect.\n";
		#endif
			fd_set Write, Err;
			TIMEVAL Timeout;
			int TimeoutSec = 5; // timeout after 5 seconds

			FD_ZERO(&Write);
			FD_ZERO(&Err);
			FD_SET(tcpinterface::sConnect, &Write);
			FD_SET(tcpinterface::sConnect, &Err);

			Timeout.tv_sec = TimeoutSec;
			Timeout.tv_usec = 0;

			iResult = select(0,         //ignored
				NULL,      //read
				&Write,    //Write Check
				&Err,      //Error Check
				&Timeout);
			if (iResult == 0)
			{
				std::cout << "Connect Timeout (" << TimeoutSec << " Sec).\n";
				system("pause");
				return 1;

			}
			else
			{
				if (FD_ISSET(tcpinterface::sConnect, &Write))
				{
					std::cout << "Had to Re-Process, but Connected!\n";
				}
				if (FD_ISSET(tcpinterface::sConnect, &Err))
				{
					std::cout << "Select error.\n";
					//system("pause");
					return 0;
				}
			}
		}
		else
		{
			std::cout << "Failed to connect!" << std::endl;
			MessageBox(hWnd, L"Failed to connect to Server!", L"Notice",
				MB_OK | MB_ICONINFORMATION);
			WSACleanup();
			return 0;
		}
	}
	closed = false;
	std::cout << "Connected!\n";
	return 1;
}

void tcpinterface::w_lock()
{
	WaitForSingleObject(writeMutex, INFINITE);
}

void tcpinterface::w_unlock()
{
	ReleaseMutex(writeMutex);
}
