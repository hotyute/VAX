#include "packets.h"
#include "clinc2.h"
#include "tools.h"
#include "usermanager.h"

void sendPositionUpdates(User &user) {
	auto out = BasicStream(20);
	out.create_frame_var_size(CONTROLLER_POS_UPDATE);
	double lat = user.getLatitude();
	double lon = user.getLongitude();
	const long long latitude = *reinterpret_cast<long long*>(&lat);
	const long long longitude = *reinterpret_cast<long long*>(&lon);
	out.write_qword(latitude);
    out.write_qword(longitude);
	out.end_frame_var_size();
	intter->sendMessage(&out);
}

void sendPingPacket() {
	BasicStream out = BasicStream(2);
	out.create_frame(_PING);
	intter->sendMessage(&out);
}

void sendUserMessage(int frequency, const std::string& to, const std::string& message) {
	BasicStream out = BasicStream(512);
	out.create_frame_var_size_word(_USER_MESSAGE);
	out.write_string(to.c_str());
	out.write_3byte(frequency);//99998 = 199.998
	out.write_string(message.c_str());
	out.end_frame_var_size_word();
	intter->sendMessage(&out);
}

void sendPrivateMessage(const std::string& to, const std::string& message) {
	BasicStream out = BasicStream(512);
	out.create_frame_var_size_word(_PRIVATE_MESSAGE);
	out.write_string(to.c_str());
	out.write_string(message.c_str());
	out.end_frame_var_size_word();
	intter->sendMessage(&out);
}

void sendFlightPlanRequest(Aircraft& user_for) {
	BasicStream out = BasicStream(5);
	out.create_frame(_FLIGHT_PLAN_REQ);
	out.write_short(user_for.getUserIndex());
	out.write_short(user_for.getFlightPlan()->cycle);
	intter->sendMessage(&out);
}

void sendDisconnect() {
	BasicStream out = BasicStream(2);
	out.create_frame(_DISCONNECT_PACKET);
	out.write_byte(0);
	intter->sendMessage(&out);
}

void sendFlightPlan(Aircraft& user) {
	BasicStream out = BasicStream(256);
	FlightPlan& fp = *user.getFlightPlan();
	out.create_frame_var_size_word(_SEND_FLIGHT_PLAN);
	out.write_short(fp.cycle);
	out.write_short(user.getUserIndex());
	out.write_byte(fp.flightRules);
	out.write_string(fp.squawkCode.c_str());
	out.write_string(fp.departure.c_str());
	out.write_string(fp.arrival.c_str());
	out.write_string(fp.alternate.c_str());
	out.write_string(fp.cruise.c_str());
	out.write_string(fp.acType.c_str());
	out.write_string(fp.scratchPad.c_str());
	out.write_string(fp.route.c_str());
	out.write_string(fp.remarks.c_str());
	out.end_frame_var_size_word();
	intter->sendMessage(&out);
}

void sendPrimFreq() {
	BasicStream out = BasicStream(8);
	out.create_frame(_PRIMARY_FREQ);
	out.write_byte(0);
	out.write_3byte(USER->userdata.frequency[0]);
	out.write_3byte(USER->userdata.frequency[1]);
	intter->sendMessage(&out);
}

void sendTempData(Aircraft& user, std::string& assembly, const void* data, ...) {
	BasicStream out = BasicStream(256);
	out.create_frame_var_size_word(_TEMP_DATA);
	out.write_string(assembly.c_str());
	va_list args;
	va_start(args, data);
	int header = va_arg(args, int);
	for (int i_11_ = assembly.length() - 1; i_11_ >= 0; i_11_--) 
	{
		if (assembly.at(i_11_) == 's')
			out.write_string(va_arg(args, std::string).c_str());
		else if (assembly.at(i_11_) == 'l')
			out.write_qword(va_arg(args, long long));
		else
			out.write_int(va_arg(args, int));
	}
	va_end(args);
	out.write_int(header);
	out.end_frame_var_size_word();
	intter->sendMessage(&out);
}