#include "clinc2.h"

#include "events.h"
#include "config.h"
#include "usermanager.h"

tcpinterface* intter = new tcpinterface();

static int packetSizes[256][2] = {
	{10, 8},
	{13, 0},
	{9, -1},
	{14, 36},
	{12, 2},
	{16, 3},
	{11, -2},
	{17, -2}
};

tcpinterface::tcpinterface() {
	this->out_stream = new Stream(5000);
}

DWORD WINAPI tcpinterface::staticStart(void* param) {
	tcpinterface* tcp = (tcpinterface*) param;
	return tcp->run();
}
int nBytesReceived = 0;
bool fBreak = false;
TIMEVAL timeout1;
int TimeoutSec1 = 30; //
fd_set rfds;
int retval;

DWORD tcpinterface::run() {
	memset(tcpinterface::message, 0, 5000);
	timeout1.tv_sec = TimeoutSec1;
	timeout1.tv_usec = 0;
	bool closed = false;
	while (!quit) {
		ZeroMemory(message, sizeof(message));
		fBreak = false;
		tcpinterface::packetType = -1;
		tcpinterface::packetSize = -3;
		int size1 = 0;

		FD_ZERO(&rfds);
		FD_SET(tcpinterface::sConnect, &rfds);

		retval = select(tcpinterface::sConnect+1,&rfds,0,0,&timeout1);

		while (!fBreak) {
			if(FD_ISSET(tcpinterface::sConnect, &rfds))
			{
				nBytesReceived = recv(tcpinterface::sConnect, message, 5000, 0);
				if(nBytesReceived <= 0) 
				{
					closed = true;
					printf("Connection was closed by remote person or timeout exceeded 60 seconds\n");
					break;
				}

				if (nBytesReceived == SOCKET_ERROR)
					break;

				if(nBytesReceived == 0) 
					continue;

				if (tcpinterface::hand_shake) 
				{
					if (tcpinterface::current_op == 45) 
					{
						if (nBytesReceived >= 11) 
						{
							Stream &in = Stream(11);
							in.currentOffset = 0;
							memcpy(in.buffer, message, 11);
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
								Event &position_updates = PositionUpdates();
								position_updates.eAction.setTicks(0);
								event_manager1->addEvent(&position_updates);
								tcpinterface::hand_shake = false;
								fBreak = true;
							}
						}
					}
				} 
				else 
				{
					int available = nBytesReceived;
					int offset = 0;
					packetType = (unsigned char)(message[offset++]);
					available--;
					if (packetType != -1) 
					{
						for(int j = 0; j < 256; j++) 
						{
							if (packetSizes[j][0] == packetType) 
							{
								packetSize = packetSizes[j][1];
								break;
							}
						}
						if (packetSize == -1) 
						{
							if (available >= 1) 
							{
								packetSize = (unsigned char)message[offset++];
								available--;
							}
						} 
						else if (packetSize == -2) 
						{
							if (available >= 2) 
							{

								int firstByte = (((unsigned char)message[offset++]) << 8);
								packetSize = firstByte + (unsigned char)message[offset++];
								available -= 2;
							}
						} 
						else if (packetSize == -3) 
						{
							//packetSize = available; //Uncomment to auto buffer
						}
#ifdef _DEBUG
						std::cout << "Packet_Id: " << (int)packetType << ", Packet_Size: " << packetSize << ", Bytes_Ava: " << available << std::endl;
#endif
						if (available >= packetSize) 
						{
							Stream &stream_in = Stream(tcpinterface::packetSize);
							stream_in.currentOffset = 0;
							memcpy(stream_in.buffer, message + offset, packetSize);
							//handle
							decodePackets(tcpinterface::packetType, stream_in);
							fBreak = true;
						}
					}
				}
			}
			FD_ZERO(&rfds);
			FD_SET(tcpinterface::sConnect, &rfds);

			retval = select(tcpinterface::sConnect+1,&rfds,0,0,&timeout1);
		}
		if (retval == SOCKET_ERROR)
		{
			//do somethin
		}
	}
	return 0;
}

void tcpinterface::sendMessage(Stream *stream) {
	if(stream->currentOffset == 0) {
		printf("Can't flush empty stream o.O\n");
		return;
	}

	DWORD what = send(tcpinterface::sConnect, stream->buffer, stream->currentOffset, NULL);
	stream->currentOffset = 0;
}

void tcpinterface::startT(HWND hWnd) {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) staticStart, (void*) this, 0, NULL);
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
		if(iError == WSAEWOULDBLOCK)
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
			if(iResult == 0)
			{
				std::cout << "Connect Timeout (" << TimeoutSec << " Sec).\n";
				system("pause");
				return 1;

			}
			else
			{
				if(FD_ISSET(tcpinterface::sConnect, &Write))
				{
					std::cout << "Connected!\n";
				}
				if(FD_ISSET(tcpinterface::sConnect, &Err))
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
	std::cout << "Connected!\n";
	return 1;
}