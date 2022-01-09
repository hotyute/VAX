#include "packets.h"
#include "clinc2.h"
#include "tools.h"

void sendPositionUpdates(User &user) {
	Stream &out = Stream(20);
	out.createFrameVarSize(CONTROLLER_POS_UPDATE);
	double lat = user.getLatitude();
	double lon = user.getLongitude();
	long long latitude = *(long long *)&lat;
	long long longitude = *(long long *)&lon;
	out.writeQWord(latitude);
    out.writeQWord(longitude);
	out.endFrameVarSize();
	intter->sendMessage(&out);
}

void sendPingPacket() {
	Stream &out = Stream(2);
	out.createFrame(_PING);
	intter->sendMessage(&out);
}

void sendUserMessage(int frequency, std::string to, std::string message) {
	Stream &out = Stream(512);
	out.createFrameVarSizeWord(_USER_MESSAGE);
	out.writeString((char*)to.c_str());
	out.writeDWord(frequency);//99998 = 199.998
	out.writeString((char*)message.c_str());
	out.endFrameVarSizeWord();
	intter->sendMessage(&out);
}

void sendPrivateMessage(std::string to, std::string message) {
	Stream& out = Stream(512);
	out.createFrameVarSizeWord(_PRIVATE_MESSAGE);
	out.writeString((char*)to.c_str());
	out.writeString((char*)message.c_str());
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
	out.writeByte(0);
	intter->sendMessage(&out);
}

void sendFlightPlan(Aircraft& user) {
	Stream& out = Stream(256);
	FlightPlan& fp = *user.getFlightPlan();
	out.createFrameVarSizeWord(_SEND_FLIGHT_PLAN);
	out.writeWord(fp.cycle);
	out.writeWord(user.getUserIndex());
	out.writeByte(fp.flightRules);
	out.writeString((char*)fp.squawkCode.c_str());
	out.writeString((char*)fp.departure.c_str());
	out.writeString((char*)fp.arrival.c_str());
	out.writeString((char*)fp.alternate.c_str());
	out.writeString((char*)fp.cruise.c_str());
	out.writeString((char*)fp.acType.c_str());
	out.writeString((char*)fp.scratchPad.c_str());
	out.writeString((char*)fp.route.c_str());
	out.writeString((char*)fp.remarks.c_str());
	out.endFrameVarSizeWord();
	intter->sendMessage(&out);
}

void sendPrimFreq() {
	Stream& out = Stream(6);
	out.createFrame(_PRIMARY_FREQ);
	out.writeByte(0);
	out.writeDWord(USER->userdata.frequency[0]);
	intter->sendMessage(&out);
}