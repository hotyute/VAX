#include "calc_cycles.h"

DWORD __stdcall CalcThread1(LPVOID)
{
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	boost::posix_time::time_duration time;

	boost::posix_time::ptime curTime1 = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime curTime2 = boost::posix_time::microsec_clock::local_time();

	while (true)
	{
		start = boost::posix_time::microsec_clock::local_time();

		if (boost::posix_time::time_duration(boost::posix_time::microsec_clock::local_time()
			- curTime1).total_milliseconds() >= 10000)
		{
			sendPingPacket();
			curTime1 = boost::posix_time::microsec_clock::local_time();
		}

		//code here
		update();
		CalculateCollisions();
		CalcDepartures();
		CalcControllerList();

		end = boost::posix_time::microsec_clock::local_time();

		time = (end - start);
		long long time1 = 100L;
		long long time2 = time1 - time.total_milliseconds();
		if (time2 < 1) {
			time2 = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time2));

	}
	return 0;
}

void update()
{
}

void CalculateCollisions() {
	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			Aircraft* acf1 = iter->second;
			if (acf1) {
				Aircraft& aircraft1 = *acf1;
				for (auto iter2 = acf_map.begin(); iter2 != acf_map.end(); iter2++) {
					Aircraft* acf2 = iter2->second;
					if (acf2 && acf2 != acf1) {
						Aircraft& aircraft2 = *acf2;

						//Check What Runway Aircraft1
					}
				}
			}
		}
	}

	if (Collision_Map.size() > 0) {
		for (auto iter = Collision_Map.begin(); iter != Collision_Map.end(); iter++) {
			// iterator->first = key
			Collision* col = iter->second;
		}
	}
}

void CalcDepartures() {
	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			Aircraft* acf1 = iter->second;
			if (acf1) {
				FlightPlan& fp = *acf1->getFlightPlan();
				std::string callsign = acf1->getCallsign();

				if (!departures.count(callsign)) {
					std::vector<std::string> points = split(fp.route, " .");

					//check if the aircraft is departing from this airport
					if (boost::iequals(fp.departure, icao) && !icao.empty()) {
						std::vector<std::string> new_points;

						int max_points = 2;
						while (max_points > 0 && points.size() > 0) {
							new_points.push_back(pop_front(points));
							max_points--;
						}
						departures.emplace(callsign, new_points);
						renderDepartures = true;
					}
				}
				else
				{
					//check for removal
				}
			}
		}
	}
}

void CalcControllerList() {
	if (controller_map.size() > 0)
	{
		for (int i = 0; i < 9; i++)
		{

		}
	}
}

void check_add_ctrl_list(Controller& controller)
{
	std::string callsign = controller.getCallsign();

	if (dist(USER->getLatitude(), USER->getLongitude(), controller.getLatitude(), controller.getLongitude()) <= USER->getVisibility())
	{
		if (!obs_list.count(callsign) && controller.getIdentity()->controller_position == 0)
		{

			std::vector<std::string> data;

			data.push_back(std::to_string(controller.getIdentity()->controller_position));
			data.push_back("1A");
			data.push_back(frequency_to_string(controller.frequency[0]));
			data.push_back(std::to_string(controller.getIdentity()->controller_rating));

			add_to_ctrl_list(callsign, data, obs_list);
			add_to_qlctrl_list(callsign, data, ql_obs_list);

			if (obs_list.empty())
				controller_list_box->addLineTop("------------OBSERVER----------", CHAT_TYPE::MAIN);

			renderDrawings = true;

		}
		else if (!del_list.count(callsign) && controller.getIdentity()->controller_position == 1)
		{
			//if (del_list.empty())
			//	controller_list_box->addLine("------------DELIVERY----------", CHAT_TYPE::MAIN);

			/*std::vector<std::string> data;

			data.push_back(std::to_string(ctlr->getIdentity()->controller_position));
			data.push_back("1A");
			data.push_back(frequency_to_string(ctlr->frequency[0]));
			data.push_back(std::to_string(ctlr->getIdentity()->controller_rating));

			add_to_ctrl_list(callsign, data, del_list);*/

			renderDrawings = true;

		}
	}
	else
	{
		//check for removal
	}
}

void add_to_ctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, ChatLine*>& store)
{
	ChatLine* c = new ChatLine("", CHAT_TYPE::MAIN);
	std::string controller_user = "";
	for (size_t i = 0; i < 7; i++)
	{
		if (i < data[1].length())
			controller_user += data[1][i];
		else
			controller_user += " ";
	}
	for (size_t i = 0; i < 16; i++)
	{
		if (i < callsign.length())
			controller_user += callsign[i];
		else
			controller_user += " ";
	}
	controller_user += data[2];
	c->setText(controller_user);

	controller_list_box->resetReaderIdx();

	if (atodd(data[3]) == 10 || atodd(data[3]) == 11)
		c->setType(CHAT_TYPE::SUP_POS);

	controller_list_box->addLineTop(c);
	controller_list_box->prepare();

	store.emplace(callsign, c);
}

void add_to_qlctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, ChatLine*>& store)
{
	ChatLine* c = new ChatLine("", CHAT_TYPE::MAIN);
	std::string controller_user = "";
	for (size_t i = 0; i < 4; i++)
	{
		if (i < data[1].length())
			controller_user += data[1][i];
		else
			controller_user += " ";
	}
	int white_space = 10 - callsign.size();

	while (white_space > 0)
	{
		controller_user += " ";
		--white_space;
	}

	controller_user += callsign;

	c->setText(controller_user);

	qlc_list_box->resetReaderIdx();

	if (atodd(data[3]) == 10 || atodd(data[3]) == 11)
		c->setType(CHAT_TYPE::SUP_POS);

	qlc_list_box->addLineTop(c);
	qlc_list_box->prepare();

	store.emplace(callsign, c);
}

void check_del_ctrl_list(Controller& controller)
{
	std::string callsign = controller.getCallsign();
	switch (controller.getIdentity()->controller_position)
	{
	case 0:
	{
		ChatLine* c = obs_list[callsign];
		if (c)
		{
			remove_ctrl_list(c);
			obs_list.erase(callsign);
		}

		ChatLine* c2 = ql_obs_list[callsign];
		if (c2)
		{
			//printf("%s", c2->getText().c_str());
			remove_qlctrl_list(c2);
			ql_obs_list.erase(callsign);
		}
	}
	break;
	case 1:
	{
		ChatLine* c = del_list[callsign], * c2 = ql_del_list[callsign];
		if (c)
		{
			remove_ctrl_list(c);
			del_list.erase(callsign);
		}
		if (c2)
		{
			remove_qlctrl_list(c2);
			ql_del_list.erase(callsign);
		}
	}
	break;

	}
}

void refresh_ctrl_list()
{
	if (controller_map.size() > 0)
	{
		for (auto iter = controller_map.begin(); iter != controller_map.end(); iter++)
		{
			Controller& controller = *iter->second;
			if (dist(USER->getLatitude(), USER->getLongitude(), controller.getLatitude(), controller.getLongitude()) > USER->getVisibility())
			{
				check_del_ctrl_list(controller);
			}
		}
	}
}

void clear_ctrl_list(std::unordered_map<std::string, ChatLine*>& store)
{
	auto it = store.begin();
	while (it != store.end())
	{
		remove_ctrl_list((*it).second);
		it = store.erase(it);
	}
}

void clear_qlctrl_list(std::unordered_map<std::string, ChatLine*>& store)
{
	auto it = store.begin();
	while (it != store.end())
	{
		remove_qlctrl_list((*it).second);
		it = store.erase(it);
	}
}

void clear_ctrl_list()
{
	clear_ctrl_list(obs_list);
	clear_ctrl_list(del_list);
	clear_qlctrl_list(ql_obs_list);
	clear_qlctrl_list(ql_del_list);
}

void remove_ctrl_list(ChatLine* c)
{
	controller_list_box->removeLine(c);
	renderDrawings = true;
}

void remove_qlctrl_list(ChatLine* c)
{
	printf("%s", c->getText().c_str());
	qlc_list_box->removeLine(c);
	renderDrawings = true;
}
