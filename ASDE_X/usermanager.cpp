#include "usermanager.h"
#include "main.h"
#include "renderer.h"
#include "tools.h"

std::vector<User*> userStorage1;
std::unordered_map<std::string, User*> users_map;

Controller* USER = new Controller("(NOT_LOGGED)", 11, 0);

void decodePackets(int opCode, Stream& stream) {
	if (opCode == 10) {
		//update Cycle Change
		USER->setUpdateTime(stream.readQWord());
	}
	if (opCode == 13) {
		//ping packet
	}
	if (opCode == 9) {
		//create new user packet
		int index = stream.readUnsignedWord();
		int type = stream.readUnsignedByte();
		char callSign1[1024], full_name[1024], username[1024];
		stream.readString(callSign1);
		stream.readString(username);
		stream.readString(full_name);
		User* user1 = nullptr;
		if (type == CONTROLLER_CLIENT) 
		{

		}
		else if (type == PILOT_CLIENT) 
		{
			char acfTitle[1024];
			stream.readString(acfTitle);
			char trans_code[1024];
			stream.readString(trans_code);
			int squawkMode = stream.readUnsignedByte();
			Aircraft* aircraft1 = new Aircraft(callSign1, 0, 0);
			user1 = (User*)aircraft1;
			aircraft1->lock();
			aircraft1->setCallsign(callSign1);
			aircraft1->setUpdateFlag(ACF_CALLSIGN, true);
			aircraft1->setUpdateFlag(ACF_COLLISION, true);
			aircraft1->setMode(squawkMode);
			aircraft1->setSquawkCode(trans_code);
			aircraft1->unlock();
			addAircraftToMirrors(aircraft1);
			AcfMap[aircraft1->getCallsign()] = aircraft1;
		}
		user1->getIdentity()->login_name = full_name;
		user1->getIdentity()->username = username;

		users_map.emplace(user1->getCallsign(), user1);
		user1->setUserIndex(index);
		userStorage1[index] = user1;

	}
	if (opCode == 12) {//delete user packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		if (user1 != NULL) {
			userStorage1[index] = NULL;
			int type = user1->getIdentity()->type;
			if (type == PILOT_CLIENT) 
			{
				users_map.erase(user1->getCallsign());
			}
			else if (type == CONTROLLER_CLIENT) 
			{

			}
			delete user1;
		}
	}
	if (opCode == 14) {
		//Pilot  Update Packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		long long lat = stream.readQWord();
		long long lon = stream.readQWord();
		double latitude = *(double*)&lat;
		double longitude = *(double*)&lon;
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
		if (user1 != NULL && user1->getIdentity()->type == CLIENT_TYPES::PILOT_CLIENT) {
			Aircraft* cur = (Aircraft*)user1;
			cur->lock();
			cur->setHeavy(false);
			cur->setLatitude(latitude);
			cur->setLongitude(longitude);
			cur->setSpeed((double)groundSpeed);
			cur->setHeading(heading);
			cur->unlock();
		}
		user1->handleMovement(latitude, longitude);
	}
	if (opCode == 16) {
		int index = stream.readUnsignedWord();
		int mode = stream.readUnsignedByte();
		User* user1 = userStorage1.at(index);
		if (user1 != NULL) {
			CLIENT_TYPES type = user1->getIdentity()->type;
			if (type == CLIENT_TYPES::PILOT_CLIENT) {
				Aircraft* acf = (Aircraft*)user1;
				if (acf != NULL) {
					acf->setMode(mode);
				}
			}
		}
	}

	if (opCode == 11) {// recieve message
		int index = stream.readUnsignedWord();
		char msg[2048];
		stream.readString(msg);
		User* user1 = userStorage1.at(index);
		if (user1 != NULL) {
			main_chat_box->addLine(user1->getIdentity()->callsign + std::string(": ") + msg, CHAT_TYPE::MAIN);
			renderDrawings = true;
		}
	}

	if (opCode == 17) {//update flight plan packet
		int index = stream.readUnsignedWord();
		User* user1 = userStorage1.at(index);
		int cur_cycle = stream.readUnsignedWord();
		int type = stream.readUnsignedByte();
	#ifdef _DEBUG
		std::cout << "cycle: " << cur_cycle << '\n' << std::endl;
	#endif
		if (type == PILOT_CLIENT) {
			int fr = stream.readUnsignedByte();
			char assigned_squawk[5], departure[5], arrival[5], alternate[5], cruise[6], ac_type[8], scratch[6], route[128], remarks[128];
			stream.readString(assigned_squawk);
			stream.readString(departure);
			stream.readString(arrival);
			stream.readString(alternate);
			stream.readString(cruise);
			stream.readString(ac_type);
			stream.readString(scratch);
			stream.readString(route);
			stream.readString(remarks);

			if (user1 != NULL) {
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
					if (opened_fp == user1 && cur_cycle) {
						Load_FlightPlan_Interface(-1, -1, acf, true);
					}
				}
			}
		}
	}
}