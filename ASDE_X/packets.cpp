#include "packets.h"
#include "clinc2.h"
#include "tools.h"

void sendPositionUpdates(User &user) {
	Stream &out = Stream(20);
	out.createFrameVarSize(CONTROLLER_POS_UPDATE);
	out.writeWord(user.getUserIndex());
	double lat = user.getLatitude();
	double lon = user.getLongitude();
	long long latitude = *(long long *)&lat;
	long long longitude = *(long long *)&lon;
	out.writeQWord(latitude);
    out.writeQWord(longitude);
	out.endFrameVarSize();
	intter->sendMessage(&out);
}

void sendPingPacket(User &user) {
	Stream &out = Stream(20);
	out.createFrame(_PING);
	intter->sendMessage(&out);
}

void sendUserMessage(User &to, std::string message) {
	Stream &out = Stream(512);
	out.createFrameVarSizeWord(_USER_MESSAGE);
	out.writeWord(USER->getUserIndex());
	out.writeWord(to.getUserIndex());
	char *msg = s2ca1(message);
	out.writeString(msg);
	out.endFrameVarSizeWord();
	intter->sendMessage(&out);
}

void sendPrivateMessage(User& to, std::string message) {
	Stream& out = Stream(512);
	out.createFrameVarSizeWord(_PRIVATE_MESSAGE);
	out.writeWord(USER->getUserIndex());
	out.writeWord(to.getUserIndex());
	char* msg = s2ca1(message);
	out.writeString(msg);
	out.endFrameVarSizeWord();
	intter->sendMessage(&out);
}

void sendFlightPlanRequest(Aircraft& user_for) {
	Stream& out = Stream(5);
	out.createFrame(_FLIGHT_PLAN_REQ);
	out.writeWord(user_for.getUserIndex());
	out.writeWord(user_for.getFlightPlan()->cycle);
	intter->sendMessage(&out);
}

void sendDisconnect() {
	Stream &out = Stream(2);
	out.createFrame(_DISCONNECT_PACKET);
	out.writeWord(USER->getUserIndex());
	intter->sendMessage(&out);
}