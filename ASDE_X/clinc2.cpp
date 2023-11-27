#include "clinc2.h"

#include <thread>
#include <iostream>
#include <chrono>

#include "config.h"
#include "interfaces.h"
#include "main.h"
#include "usermanager.h"

using namespace std::chrono_literals;

tcpinterface* intter = new tcpinterface();

/* Use Official Packet Output App to update this. */
const int packetSizes[256] =
{
-3, -3, -3, -3, -3, -3, -3, -1, -1, -2, 8, -2, 2, 0, 36,
-2, 3, -2, 19, 4, -1, 7, -2, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
-3,
};

tcpinterface::tcpinterface() {
	this->in_stream = new BasicStream();
	timeout1.tv_sec = TimeoutSec1;
	timeout1.tv_usec = 0;
	sConnect = INVALID_SOCKET;
}

DWORD WINAPI tcpinterface::staticStart(void* param) {
	tcpinterface* tcp = (tcpinterface*)param;
	return tcp->run();
}

DWORD tcpinterface::run() {
	while (!quit)
	{

		FD_ZERO(&rfds);
		FD_SET(sConnect, &rfds);


		retval = select(sConnect + 1, &rfds, 0, 0, &timeout1);

		if (FD_ISSET(sConnect, &rfds))
		{
			nBytesReceived = in_stream->add_data(sConnect);

			if (nBytesReceived == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error == WSAECONNABORTED || error == WSAENOTSOCK
					|| error == WSAECONNRESET || error == WSAETIMEDOUT)
				{
					disconnect();
					in_stream->clear();
					closed = true;
					if (error == WSAETIMEDOUT)
						printf("Connection timeout exceeded 11 seconds\n");
					else if (error != WSAENOTSOCK)
						printf("Connection was closed by remote person or timeout exceeded 60 seconds\n");
				}
				else
				{
					printf("Unhandled_Error: %d\n", error);
				}
				break;
			}

			if (nBytesReceived == 0)
				continue;

			BasicStream& in = *in_stream;

			if (tcpinterface::hand_shake)
			{
				if (tcpinterface::current_op == 45)
				{
					in.mark_position();
					if (nBytesReceived >= 1)
					{
						const int loginCode = in.read_unsigned_byte();

						if (loginCode == 1)
						{
							if (in.available() >= 10)
							{
								//setIndex
								//setUpdateTimeinMillis
								//sendUpdates
								const int index = in.read_unsigned_short();
								const long long updateTimeInMillis = in.readQWord();
								USER->setUserIndex(index);
								userStorage1[index] = USER;
								USER->setUpdateTime(updateTimeInMillis);
								if (!this->position_updates)
								{
									this->position_updates = new PositionUpdates();
									position_updates->eAction.setTicks(0);
									event_manager1->addEvent(position_updates);
								}
								else if (this->position_updates->eAction.paused)
								{
									position_updates->eAction.setTicks(0);
									this->position_updates->toggle_pause();
								}
								event_manager1->addEvent(position_updates);
								tcpinterface::hand_shake = false;
								in.delete_marked_block();
							}
							else
							{
								in.reset();
							}
						}
						else
						{
							switch (loginCode)
							{
							case 2:
							{
								sendErrorMessage("Invalid protocol Version.");
								if (connected)
									closesocket(sConnect);
								in.delete_marked_block();
								break;
							}
							default:
							{
								in.delete_marked_block();
								break;
							}
							}
						}
					}
					else
					{
						in.reset();
					}
				}
			}

			if (!hand_shake)
			{
				if (in.available() > 0)
				{
					decode(in);
				}
			}
		}
	}
	if (retval == SOCKET_ERROR)
	{
		//do somethin
	}
	return 0;
}

void decode(BasicStream& in)
{
	while (in.available() > 0)
	{
		in.mark_position();
		int opCode = in.read_unsigned_byte(), length = -3;
		if (opCode != -1)
		{
			length = packetSizes[opCode];
			if (length == -1)
			{
				if (in.available() >= 1)
				{
					length = in.read_unsigned_byte();
				}
				else
				{
					in.reset();
					break;
				}
			}
			else if (length == -2)
			{
				if (in.available() >= 2)
				{
					length = in.read_unsigned_short();
				}
				else
				{
					in.reset();
					break;
				}
			}
			else if (length == -3)
			{
#ifdef _DEBUG
				std::cout << "Unhandled Packet_Id!! : [" << (int)opCode << ", Packet_Size: "
					<< length << ", Bytes_Ava: " << in.available() << "] ... Skipping" << std::endl;
#endif
				length = in.available();
			}
#ifdef _DEBUG
			//std::cout << "Packet_Id: " << (int)opCode << ", Packet_Size: " << length << ", Bytes_Ava: " << in.available() << std::endl;
#endif
			if (in.available() >= length)
			{
				//handle
				decodePackets(opCode, in);
				in.delete_marked_block();
			}
			else
			{
				in.reset();
				break;
			}
		}
	}
}

void tcpinterface::sendMessage(BasicStream* stream) {
	std::lock_guard<std::mutex> lock(tcpinterface::writeMutex);
	if (stream->get_index() == 0) {
		printf("Can't flush empty stream o.O\n");
		return;
	}

	send_data(tcpinterface::sConnect, std::vector<char>(stream->data, stream->data + stream->index));
	stream->clear();
}

void tcpinterface::send_data(SOCKET clientSocket, const std::vector<char>& buffer) {
	size_t totalSent = 0;
	size_t remaining = buffer.size();

	while (totalSent < buffer.size()) {
		const int sent = send(clientSocket, buffer.data() + totalSent, static_cast<int>(remaining), 0);
		if (sent == SOCKET_ERROR) {
			std::cerr << "Error sending broadcast message: " << WSAGetLastError() << std::endl;
			break;
		}

		printf("sent: %d\n", sent);

		totalSent += sent;
		remaining -= sent;
	}
}

void tcpinterface::startT(HWND hWnd) {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)staticStart, (void*)this, 0, NULL);
}

int tcpinterface::disconnect_socket() {
	int iResult = shutdown(tcpinterface::sConnect, 0x01);
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

	struct hostent* to;
	/* generate address */
	if ((to = gethostbyname(saddr.c_str())) == NULL)
	{
		printf("gethostbyname() error...\n");
		MessageBox(hWnd, L"Host Name Error!", L"Notice",
			MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	SOCKADDR_IN addr;

	int addrlen = sizeof(addr);

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);

	//addr.sin_addr.s_addr = inet_addr(saddr.c_str());
	memcpy(&addr.sin_addr, to->h_addr_list[0], to->h_length);

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
