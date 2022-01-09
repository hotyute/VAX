#include "usermanager.h"
#include "main.h"
#include "renderer.h"
#include "tools.h"
#include "calc_cycles.h"

std::vector<User*> userStorage1;
std::unordered_map<std::string, User*> users_map;

Controller* USER = new Controller("(NOT_LOGGED)", 0, 0);
User* ASEL = nullptr;

void decodePackets(int opCode, Stream& stream) {
	if (opCode == 7) {
		char msg[256];
		stream.readString(msg);
		sendSystemMessage("Server: " + std::string(msg));
	}

	if (opCode == 8) {
		char wx[256];
		stream.readString(wx);
	}

	if (opCode == 9) {
		//create new user packet
		int index = stream.readUnsignedWord();
		CLIENT_TYPES type = static_cast<CLIENT_TYPES>(stream.readUnsignedByte());
		char callSign1[1024], full_name[1024], username[1024];
		stream.readString(callSign1);
		stream.readString(username);
		stream.readString(full_name);
		int vis_range = stream.readUnsignedWord();
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		User* user1 = nullptr;
		if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
		{
			Controller* controller1 = new Controller(callSign1, 0, 0);
			controller1->getIdentity()->controller_rating = stream.readUnsignedByte();
			controller1->getIdentity()->controller_position = static_cast<POSITIONS>(stream.readUnsignedByte());
			controller1->userdata.frequency[0] = stream.readDWord();
			user1 = (User*)controller1;
			controller1->lock();
			controller1->setCallsign(callSign1);
			controller1->unlock();
			controller_map[callSign1] = controller1;
		}
		else if (type == CLIENT_TYPES::PILOT_CLIENT)
		{
			char acfTitle[1024];
			stream.readString(acfTitle);
			char trans_code[1024];
			stream.readString(trans_code);
			int m = stream.readUnsignedByte();
			long long hash = stream.readQWord();
			unsigned long long num2 = hash >> 22;
			unsigned int num3 = hash >> 12 & 1023u;
			unsigned int num4 = hash >> 2 & 1023u;
			double pitch = num2 / 1024.0 * -360.0;
			double roll = num3 / 1024.0 * -360.0;
			double heading = num4 / 1024.0 * 360.0;

			int squawkMode = m >> 4;
			bool heavy = (m & 0xf) == 1;

			//TODO SEND THE BLOODY HEADINGS PITCH AND ROLL!
			Aircraft* aircraft1 = new Aircraft(callSign1, 0, 0);
			user1 = (User*)aircraft1;
			aircraft1->lock();
			aircraft1->setCallsign(callSign1);
			aircraft1->setUpdateFlag(ACF_CALLSIGN, true);
			aircraft1->setUpdateFlag(ACF_COLLISION, true);
			aircraft1->setMode(squawkMode);
			aircraft1->setSquawkCode(trans_code);
			aircraft1->setThreeFactor(heading, pitch, roll);
			aircraft1->unlock();
			addAircraftToMirrors(aircraft1);
			acf_map[callSign1] = aircraft1;
		}

		if (user1)
		{
			user1->getIdentity()->login_name = full_name;
			user1->getIdentity()->username = username;
			user1->setVisibility(vis_range);

			user1->setLatitude((*(double*)&lat));
			user1->setLongitude((*(double*)&lon));

			users_map.emplace(user1->getCallsign(), user1);
			user1->setUserIndex(index);
			userStorage1[index] = user1;

			if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				refresh_ctrl_list();
			}
		}
	}
	if (opCode == 10) {
		//update Cycle Change
		USER->setUpdateTime(stream.readQWord());
	}
	if (opCode == 12)
	{//delete user packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		if (user1)
		{
			std::string callsign = user1->getCallsign();
			CLIENT_TYPES type = user1->getIdentity()->type;

			if (type == CLIENT_TYPES::PILOT_CLIENT)
			{
				Aircraft* aircraft = (Aircraft*)user1;
				if (aircraft->collisions.size() > 0)
				{
					for (auto it = aircraft->collisions.begin(); it != aircraft->collisions.end();)
					{
						Aircraft* ac2 = it->first;
						Collision* col = it->second;
						if (ac2)
						{
							ac2->collisions.erase(aircraft);
							Collision_Map.erase(ac2);
						}
						Collision_Map.erase(aircraft);
						delete col;
						it = aircraft->collisions.erase(it);
					}
				}
			}
			else if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				//do stuff before delete
			}

			if (ASEL == user1)
				ASEL = nullptr;
			delete user1;
			userStorage1[index] = nullptr;

			//TODO Handle Collisions

			if (type == CLIENT_TYPES::PILOT_CLIENT)
			{
				acf_map.erase(callsign);
			}
			else if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				controller_map.erase(callsign);
			}

			users_map.erase(callsign);

			if (type == CLIENT_TYPES::PILOT_CLIENT)
			{
				if (departures.count(callsign))
				{
					departures.erase(callsign);
					renderDepartures = true;
				}
			}
			else if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				refresh_ctrl_list();
			}
		}
	}
	if (opCode == 13) {
		//ping packet
	}
	if (opCode == 14)
	{
		//Pilot  Update Packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		double _latitude = *(double*)&lat;
		double _longitude = *(double*)&lon;
		long long hash = stream.readQWord();
		unsigned long long num2 = hash >> 22;
		unsigned int num3 = hash >> 12 & 1023u;
		unsigned int num4 = hash >> 2 & 1023u;
		double pitch = num2 / 1024.0 * -360.0;
		double roll = num3 / 1024.0 * -360.0;
		double heading = num4 / 1024.0 * 360.0;
		int groundSpeed = stream.readUnsignedWord();
		long long alt = stream.readQWord();
		double altitude = *(double*)&alt;
		if (user1)
		{
			if (user1->getIdentity()->type == CLIENT_TYPES::PILOT_CLIENT)
			{
				Aircraft* cur = (Aircraft*)user1;
				cur->lock();
				cur->setHeavy(false);
				cur->setSpeed((double)groundSpeed);
				cur->setThreeFactor(heading, pitch, roll);
				cur->unlock();
			}
			user1->handleMovement(_latitude, _longitude);
		}
	}

	if (opCode == 18)
	{
		//Controller Update Packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		double _latitude = *(double*)&lat;
		double _longitude = *(double*)&lon;
		int flags = stream.readUnsignedByte();
		if (user1)
		{
			if (user1->getIdentity()->type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				Controller* cur = (Controller*)user1;
				cur->lock();
				cur->setOnBreak(false);
				cur->unlock();
			}
			user1->handleMovement(_latitude, _longitude);
		}
	}

	if (opCode == 15)
	{
		int index = stream.readUnsignedWord();
		int frequency = stream.readDWord();
		bool asel = stream.readUnsignedByte() == 1;
		char msg[2048];
		stream.readString(msg);
		User* user1 = userStorage1.at(index);
		if (user1)
		{
			main_chat_box->resetReaderIdx();
			ChatLine* c = new ChatLine(user1->getIdentity()->callsign + std::string(": ") + msg, CHAT_TYPE::MAIN, main_chat_box);
			main_chat_box->addLine(c);
			c->playChatSound();
			renderDrawings = true;
		}
	}

	if (opCode == 16) {
		int index = stream.readUnsignedWord();
		int mode = stream.readUnsignedByte();
		User* user1 = userStorage1.at(index);
		if (user1)
		{
			CLIENT_TYPES type = user1->getIdentity()->type;
			if (type == CLIENT_TYPES::PILOT_CLIENT)
			{
				Aircraft* acf = (Aircraft*)user1;
				if (acf)
				{
					acf->handleModeChange(mode);
					acf->setMode(mode);
				}
			}
		}
	}

	if (opCode == 11) {// recieve private message
		char _callsign[25];
		stream.readString(_callsign);
		char msg[2048];
		stream.readString(msg);
		std::string callsign = std::string(_callsign);
		capitalize(callsign);

		auto it = find(pm_callsigns.begin(), pm_callsigns.end(), callsign);
		bool unset = false;
		if (it == pm_callsigns.end())
		{
			it = find(pm_callsigns.begin(), pm_callsigns.end(), "NOT_LOGGED");
			unset = true;
		}

		if (it != pm_callsigns.end())
		{
			InterfaceFrame& frame = *frames_def[it - pm_callsigns.begin()];
			if (unset)
			{
				frame.title = "PRIVATE CHAT: " + callsign;
				pm_callsigns[it - pm_callsigns.begin()] = callsign;
			}
			DisplayBox& box = *((DisplayBox*)frame.children[PRIVATE_MESSAGE_BOX]);
			box.resetReaderIdx();
			ChatLine* c = new ChatLine(callsign + std::string(": ") + msg, CHAT_TYPE::CHAT, &box);
			box.addLine(c);
			c->playChatSound();
		}
		else
		{

		}
		renderDrawings = true;
	}

	if (opCode == 19)
	{//update visibility packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		int vis_range = stream.readUnsignedWord();
		if (user1)
		{
			user1->setVisibility(vis_range);
		}
	}

	if (opCode == 17) {//update flight plan packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		int cur_cycle = stream.readUnsignedWord();
		CLIENT_TYPES type = static_cast<CLIENT_TYPES>(stream.readUnsignedByte());
#ifdef _DEBUG
		printf("cycle: %d\n", cur_cycle);
#endif
		if (type == CLIENT_TYPES::PILOT_CLIENT) {
			int fr = stream.readUnsignedByte();
			char assigned_squawk[5], departure[5], arrival[5], alternate[5], cruise[6], ac_type[9], scratch[6], route[128], remarks[128];
			stream.readString(assigned_squawk);
			stream.readString(departure);
			stream.readString(arrival);
			stream.readString(alternate);
			stream.readString(cruise);
			stream.readString(ac_type);
			stream.readString(scratch);
			stream.readString(route);
			stream.readString(remarks);

			if (user1)
			{
				CLIENT_TYPES type = user1->getIdentity()->type;
				if (type == CLIENT_TYPES::PILOT_CLIENT) {
					Aircraft& acf = *((Aircraft*)user1);
					FlightPlan& fp = *acf.getFlightPlan();

					fp.cycle = cur_cycle;

					fp.flightRules = fr;

					fp.squawkCode = assigned_squawk;
					fp.departure = departure;
					fp.arrival = arrival;
					fp.alternate = alternate;
					fp.cruise = cruise;
					fp.acType = ac_type;
					fp.scratchPad = scratch;
					fp.route = route;
					fp.remarks = remarks;

					//TODO open Flight Plan
					if (opened_fp == user1 && cur_cycle)
					{
						int* wdata = USER->userdata.window_positions[_WINPOS_FLIGHTPLAN];
						Load_FlightPlan_Interface(wdata[0], wdata[1], acf, true);
					}

					if (fp.squawkCode != acf.getSquawkCode())
						acf.setUpdateFlag(ACF_CALLSIGN, true);
				}
			}
		}
	}

	if (opCode == 20)
	{
		int index = stream.readUnsignedWord();
		char code[20];
		stream.readString(code);
		User* user1 = userStorage1.at(index);
		if (user1)
		{
			if (is_digits(code))
			{
				CLIENT_TYPES type = user1->getIdentity()->type;

				if (type == CLIENT_TYPES::PILOT_CLIENT)
				{
					((Aircraft*)user1)->setSquawkCode(code);
					((Aircraft*)user1)->setUpdateFlag(ACF_CALLSIGN, true);
					printf("code: %s\n", code);
				}
			}
		}
	}

	if (opCode == 21)
	{
		User* subject = userStorage1.at(stream.readUnsignedWord());
		int flag = stream.readUnsignedByte();
		int freq = stream.readDWord();
		if (subject)
		{
			subject->userdata.frequency[0] = freq;
			refresh_ctrl_list();
		}
	}
}