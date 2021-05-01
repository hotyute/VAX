#include "clinc.h"
#include "tools.h"
#include "config.h"
#include "user.h"
#include "spline2.h"
#include "Ws2tcpip.h"

tcpinterface1::tcpinterface1() {

}

DWORD WINAPI tcpinterface1::staticStart(void* param) {
	tcpinterface1* intter = (tcpinterface1*)param;
	return intter->run();
}
int nBytesReceived1 = 0;
bool fBreak1 = false;

DWORD tcpinterface1::run() {
	char message[1024];
	std::string strmessage;
	while (!quit) {
		strmessage = "";
		fBreak1 = false;
		while (!fBreak1) {
			ZeroMemory(message, sizeof(message));
			nBytesReceived1 = recv(tcpinterface1::sConnect, message, sizeof(message), NULL);
			if (nBytesReceived1 == SOCKET_ERROR)
				break;
			strmessage += message;
			int length = strmessage.length();
			if (length >= 2)
			{
				if (strmessage[length - 2] == '\r' && strmessage[length - 1] == '\n')
				{
#ifdef _DEBUG
					std::cout << strmessage.c_str() << std::endl;
#endif
					fBreak1 = true;
					if (strmessage.length() != 0)
					{
						if (strmessage.substr(strmessage.length() - 1) == "\0")
						{
							strmessage = strmessage.substr(0, strmessage.length() - 1);
						}
						std::vector<std::string> array2 = split(strmessage, "\r\n");
						for (size_t i = 0; i < array2.size(); i++)
						{
							std::string text = array2[i];
							if (text.length() != 0)
							{
								std::vector<std::string> array3 = split(strmessage, ":");
								char c = array3[0][0];
								char c2 = c;
								switch (c2)
								{
								case '#':
								case '$':
								{
									if (array3[0].length() < 3)
									{
										printf("%s%s", "Invalid PDU type.", text.c_str());
										throw;
									}
									break;
								}
								default:
								{
									if (c2 != '@')
									{
										//goto IL_1058;
										break;
									}
									array3[0] = array3[0].substr(1);
									/*if (this.Field_5 != null)
									{
									this.Field_5(this, new Class_177_EventArgs<Class_233_Class_176_Object>(Class_233_Class_176_Object.Function_Class_233_Class_176_Object_26(array3), this.Field_53));
									}*/
									char header = array3[0][0];
									if (header == 'S' || header == 'N' || header == 'Y') {
										std::string callsign = array3[1];
										Aircraft* cur = AcfMap[callsign];
										unsigned int hash;
										std::stringstream ss(array3[8]);
										ss >> hash;
										unsigned int num2 = hash >> 22;
										unsigned int num3 = hash >> 12 & 1023u;
										unsigned int num4 = hash >> 2 & 1023u;
										double pitch = num2 / 1024.0 * -360.0;
										double roll = num3 / 1024.0 * -360.0;
										double heading = num4 / 1024.0 * 360.0;
										if (!cur) {
											cur = new Aircraft();
											cur->lock();
											cur->setCallsign(callsign);
											cur->setLatitude(atof(array3[4].c_str()));
											cur->setLongitude(atof(array3[5].c_str()));
											cur->setSpeed(atof(array3[7].c_str()));
											cur->setHeading(heading);
											cur->setUpdateFlag(ACF_CALLSIGN, true);
											AcfMap[callsign] = cur;
											cur->unlock();
										}
										else {
											cur->lock();
											cur->setLatitude(atof(array3[4].c_str()));
											cur->setLongitude(atof(array3[5].c_str()));
											cur->setSpeed(atof(array3[7].c_str()));
											cur->setHeading(heading);
											double result[2];
											double coords1[2][2] = { {0, 0}, {4, 4} };
											Spline2D spline(coords1);
											double what = 0.5;
											spline.getPoint(what, result);
#ifdef _DEBUG
											std::cout << result[0] << ", " << result[1] << std::endl;
#endif
											cur->unlock();
										}
									}
									break;
								}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

DWORD tcpinterface1::sendMessage(std::string message) {
	message += "\r\n";
	return send(tcpinterface1::sConnect, message.c_str(), message.length(), NULL);
}

void tcpinterface1::startT(HWND hWnd) {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)staticStart, (void*)this, 0, NULL);
}

int tcpinterface1::connectNew(HWND hWnd, std::string saddr, unsigned short port) {
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

	//addr.sin_addr.s_addr = inet_addr(saddr.c_str());
	InetPton(AF_INET, (PCWSTR)(saddr.c_str()), &addr.sin_addr.s_addr);

	addr.sin_port = htons(port);

	addr.sin_family = AF_INET;

	if (connect(tcpinterface1::sConnect, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cout << "Failed to connect!" << std::endl;
		MessageBox(hWnd, L"Failed to connect to Server!", L"Notice",
			MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	return 1;
}