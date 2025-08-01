#include "usermanager.h"

#include <bit>

#include "main.h"
#include "renderer.h"
#include "tools.h"
#include "calc_cycles.h"
#include "flightplan.h"
#include "interfaces.h"
#include "gui/widgets/ui_manager.h"
#include "gui/ui_windows/flight_plan_window.h"
#include "gui/ui_windows/private_chat_window.h"

std::vector<User*> userStorage1;
std::unordered_map<std::string, User*> users_map;

Controller* USER = new Controller("(NOT_LOGGED)", 0, 0);
User* ASEL = nullptr;

void decodePackets(int opCode, BasicStream& stream) {
	if (opCode == 7) {
		const char* msg = stream.read_string();
		sendSystemMessage("Server: " + std::string(msg));
	}

	if (opCode == 8) {
		const char* wx = stream.read_string();
	}

	if (opCode == 9) {
		//create new user packet
		int index = stream.read_unsigned_short();
		CLIENT_TYPES type = static_cast<CLIENT_TYPES>(stream.read_unsigned_byte());
		const char* callSign1 = stream.read_string();
		const char* username = stream.read_string();
		const char* full_name = stream.read_string();
		int vis_range = stream.read_unsigned_short();
		double latitude = std::bit_cast<double>(stream.readQWord());
		double longitude = std::bit_cast<double>(stream.readQWord());
		User* user1 = nullptr;
		if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
		{
			auto* controller1 = new Controller(callSign1, 0, 0);
			controller1->getIdentity()->controller_rating = stream.read_unsigned_byte();
			controller1->getIdentity()->controller_position = static_cast<POSITIONS>(stream.read_unsigned_byte());
			controller1->userdata.frequency[0] = stream.read3Byte();
			user1 = static_cast<User*>(controller1);
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
			int m = stream.read_unsigned_byte();
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
			auto* aircraft1 = new Aircraft(callSign1, 0, 0);
			user1 = static_cast<User*>(aircraft1);
			aircraft1->lock();
			aircraft1->setCallsign(callSign1);
			aircraft1->setUpdateFlag(ACF_CALLSIGN, true);
			aircraft1->setUpdateFlag(ACF_COLLISION, true);
			aircraft1->setMode(squawkMode);
			aircraft1->setHeavy(heavy);
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

			user1->setLatitude(latitude);
			user1->setLongitude(longitude);

			users_map.emplace(user1->getCallsign(), user1);
			user1->setUserIndex(index);
			userStorage1[index] = user1;

			if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				NotifyControllerListUIDirty();
			}
		}
	}
	if (opCode == 10) {
		//update Cycle Change
		USER->setUpdateTime(stream.readQWord());
	}

	if (opCode == 11) { // recieve private message
		char _senderCallsign_c[25];
		stream.readString(_senderCallsign_c);
		char msg_c[2048];
		stream.readString(msg_c);

		std::string senderCallsign = std::string(_senderCallsign_c);
		capitalize(senderCallsign);
		std::string messageContent = std::string(msg_c);

		if (UIManager::Instance().IsInitialized()) {
			std::string windowId = "PrivChat_" + senderCallsign;
			PrivateChatWindow* pmWin = UIManager::ShowOrCreateInstance<PrivateChatWindow>(
				windowId,
				-1, // No specific _WINPOS_, will center or use default
				senderCallsign // Argument for PrivateChatWindow constructor
			);

			if (pmWin) {
				pmWin->AddMessage(senderCallsign, messageContent, false);
				// UIManager::ShowOrCreateInstance already brings to front and requests focus
				// if it was just created. If it existed, BringWindowToFront was called.
				// We might want an explicit BringWindowToFront here if it was already open but not front.
				UIManager::Instance().BringWindowToFront(pmWin);
				// If the window was not focused, and the app is active, consider flashing title or taskbar icon
			}
			else if (!UIManager::Instance().IsInitialized()) {
				// This case is unlikely if UIManager is checked before calling ShowOrCreateInstance
				// but good for robustness.
				// Maybe log an error "UI not ready to display PM".
			}
		}
		else {
			// UI not ready, perhaps queue the message or log it.
			// For now, it might be lost if the UI isn't up.
			// Consider a fallback like logging to main chat box.
			// sendSystemMessage("PM from " + senderCallsign + ": " + messageContent);
		}

		// Old logic to remove:
		// auto it = find(pm_callsigns.begin(), pm_callsigns.end(), callsign);
		// ... (rest of old pm_callsigns and direct frame manipulation) ...
		// rendererFlags["drawings"] = true; // Now handled by MarkDirty in AddMessage
		return; // Ensure this packet doesn't fall through to other logic
	}


	if (opCode == 12)
	{//delete user packet
		int index = stream.read_unsigned_short();
		if (User* user1 = userStorage1.at(index))
		{
			std::string callsign = user1->getCallsign();
			CLIENT_TYPES type = user1->getIdentity()->type;

			if (type == CLIENT_TYPES::PILOT_CLIENT)
			{
				auto* aircraft = dynamic_cast<Aircraft*>(user1);
				if (!aircraft->collisions.empty())
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
					rendererFlags["renderDepartures"] = true;
				}
			}
			else if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				NotifyControllerListUIDirty();
			}
		}
	}
	if (opCode == 13) {
		//ping packet
	}
	if (opCode == 14)
	{
		//Pilot  Update Packet
		int index = stream.read_unsigned_short();
		User* user1 = userStorage1.at(index);
		double latitude = std::bit_cast<double>(stream.readQWord());
		double longitude = std::bit_cast<double>(stream.readQWord());
		long long hash = stream.readQWord();
		unsigned long long num2 = hash >> 22;
		unsigned int num3 = hash >> 12 & 1023u;
		unsigned int num4 = hash >> 2 & 1023u;
		double pitch = num2 / 1024.0 * -360.0;
		double roll = num3 / 1024.0 * -360.0;
		double heading = num4 / 1024.0 * 360.0;
		int groundSpeed = stream.read_unsigned_short();
		long long alt = stream.readQWord();
		double altitude = *reinterpret_cast<double*>(&alt);
		if (user1)
		{
			if (user1->getIdentity()->type == CLIENT_TYPES::PILOT_CLIENT)
			{
				auto* cur = dynamic_cast<Aircraft*>(user1);
				cur->lock();
				cur->setSpeed((double)groundSpeed);
				cur->setThreeFactor(heading, pitch, roll);
				cur->unlock();
			}
			user1->handleMovement(latitude, longitude);
		}
	}

	if (opCode == 15)
	{
		int index = stream.read_unsigned_short();
		int frequency = stream.read3Byte();
		bool asel = stream.read_unsigned_byte() == 1;
		char msg[2048];
		stream.readString(msg);
		if (User* user1 = userStorage1.at(index))
		{
			main_chat_box->resetReaderIdx();
			auto c = std::make_shared<ChatLine>(user1->getIdentity()->callsign + std::string(": ") + msg, CHAT_TYPE::MAIN, main_chat_box);
			main_chat_box->addLine(c);
			c->playChatSound();
			rendererFlags["drawings"] = true;
		}
	}

	if (opCode == 16) {
		int index = stream.read_unsigned_short();
		int i = stream.read_unsigned_byte();
		int mode = i >> 4, heavy = i & 0xF;
		if (User* user1 = userStorage1.at(index))
		{
			CLIENT_TYPES type = user1->getIdentity()->type;
			if (type == CLIENT_TYPES::PILOT_CLIENT)
			{
				auto* acf = dynamic_cast<Aircraft*>(user1);
				if (acf)
				{
					acf->setHeavy(heavy);
					acf->handleModeChange(mode);
					acf->setMode(mode);
				}
			}
		}
	}

	if (opCode == 17) {//update flight plan packet
		int index = stream.read_unsigned_short();
		User* user1 = userStorage1.at(index);
		int cur_cycle = stream.read_unsigned_short();
		auto type = static_cast<CLIENT_TYPES>(stream.read_unsigned_byte());
#ifdef _DEBUG
		printf("cycle: %d\n", cur_cycle);
#endif
		if (type == CLIENT_TYPES::PILOT_CLIENT) {
			int fr = stream.read_unsigned_byte();
			std::vector<std::string> vars(9);
			for (auto& var : vars) { var = stream.read_std_string(); }

			if (user1) { // user1 is the Aircraft*
				CLIENT_TYPES type = user1->getIdentity()->type;
				if (type == CLIENT_TYPES::PILOT_CLIENT) {
					Aircraft& acf = *static_cast<Aircraft*>(user1);
					FlightPlan& fp = *acf.getFlightPlan();
					fp.cycle = cur_cycle;

					fp.flightRules = fr;
					fp.updateFlightPlan(vars[0], vars[1], vars[2], vars[3], vars[4], vars[5], vars[6], vars[7], vars[8]);

					// Now, try to update an open FlightPlanWindow
					FlightPlanWindow* fpWin = UIManager::Instance().GetFlightPlanWindowForAircraft(&acf);
					if (fpWin && fpWin->visible) {
						// Check if this fpWin is for the aircraft whose FP was updated
						// This check needs to access fpWin's targetAircraft or initialCallsign
						// For simplicity, let's assume a method like IsForAircraft(Aircraft* aircraft)
						// Or directly call an update method that does the check:
						fpWin->UpdateData(&acf); // UpdateData will check if it's the right window
					}

					if (fp.squawkCode != acf.getSquawkCode()) { // This check should be here or within Aircraft::setSquawkCode
						acf.setSquawkCode(fp.squawkCode); // Use setter if it exists and handles dirty flags
						acf.setUpdateFlag(ACF_CALLSIGN, true);
					}
				}
			}
		}
	}

	if (opCode == 18)
	{
		//Controller Update Packet
		int index = stream.read_unsigned_short();
		User* user1 = userStorage1.at(index);
		double latitude = std::bit_cast<double>(stream.readQWord());
		double longitude = std::bit_cast<double>(stream.readQWord());
		int flags = stream.read_unsigned_byte();
		if (user1)
		{
			if (user1->getIdentity()->type == CLIENT_TYPES::CONTROLLER_CLIENT)
			{
				auto* cur = dynamic_cast<Controller*>(user1);
				cur->lock();
				cur->setOnBreak(false);
				cur->unlock();
			}
			user1->handleMovement(latitude, longitude);
		}
	}

	if (opCode == 19)
	{//update visibility packet
		int index = stream.read_unsigned_short();
		User* user1 = userStorage1.at(index);
		int vis_range = stream.read_unsigned_short();
		if (user1)
		{
			user1->setVisibility(vis_range);
		}
	}

	if (opCode == 20)
	{
		int index = stream.read_unsigned_short();
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
					dynamic_cast<Aircraft*>(user1)->setSquawkCode(code);
					dynamic_cast<Aircraft*>(user1)->setUpdateFlag(ACF_CALLSIGN, true);
					printf("code: %s\n", code);
				}
			}
		}
	}

	if (opCode == 21)
	{
		User* subject = userStorage1.at(stream.read_unsigned_short());
		int flag = stream.read_unsigned_byte();
		int freq = stream.read_unsigned_int();
		if (subject)
		{
			subject->userdata.frequency[0] = freq;
			NotifyControllerListUIDirty();
		}
	}

	if (opCode == 22)
	{
		User* subject = userStorage1.at(stream.read_unsigned_short());
		int index = stream.read_unsigned_short();
		ClientScript script = ClientScript(stream.read_string());
		for (int i_11_ = script.assembly.length() - 1; i_11_ >= 0; i_11_--)
		{
			if (script.assembly.at(i_11_) == 's')
				script.objects[i_11_ + 1] = stream.read_string();
			else if (script.assembly.at(i_11_) == 'l')
				script.objects[i_11_ + 1] = stream.readQWord();
			else
				script.objects[i_11_ + 1] = (int)stream.read_unsigned_int();
		}
		script.objects[0] = (int)stream.read_unsigned_int();
		//process script
		USER->registerScript(subject, index, script);
	}

	if (opCode == 23)
	{
		User* subject = userStorage1.at(stream.read_unsigned_short());
		int index = stream.read_unsigned_short();
		//process script
		USER->unregisterScript(subject, index);
	}
}