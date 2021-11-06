#include "calc_cycles.h"
#include "controller.h"

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

				std::string log = "";
				for (auto& s : logic)
				{
					if (aircraft1.on_logic(s))
					{
						log = s;
						break;
					}
				}

				if (!log.empty())
				{
					for (auto iter2 = acf_map.begin(); iter2 != acf_map.end(); iter2++)
					{

						Aircraft* acf2 = iter2->second;
						if (acf2 && acf2 != acf1) {
							if (aircraft1.collisions.find(acf2) == aircraft1.collisions.end())
							{
								Aircraft& aircraft2 = *acf2;
								if (aircraft2.collisions.find(acf1) == aircraft2.collisions.end())
								{
									if (aircraft2.near_logic(log))
									{
										Collision* collision = new Collision(acf1, acf2);
										collision->setUpdateFlag(COL_COLLISION_LINE, true);
										aircraft1.collisions.emplace(acf2, collision);
										aircraft2.collisions.emplace(acf1, collision);
										addCollisionToMirrors(collision);
										Collision_Map.emplace(acf1, collision);
										Collision_Map.emplace(acf2, collision);
									}
								}
								//Check What Runway Aircraft1
							}
						}
					}
				}

				if (aircraft1.collisions.size() > 0 && !aircraft1.isCollision())
				{
					aircraft1.setUpdateFlag(ACF_COLLISION, true);
					aircraft1.setCollision(true);
				}
				else if (aircraft1.collisions.size() <= 0 && aircraft1.isCollision())
				{
					aircraft1.setUpdateFlag(ACF_COLLISION, true);
					aircraft1.setCollision(false);
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

					if (!boost::iequals(fp.departure, icao) || icao.empty())
					{
					}
					else
					{
						std::vector<std::string> _points = departures[callsign];

						std::vector<std::string> points = split(fp.route, " .");

						std::vector<std::string> new_points;

						int max_points = 2;
						while (max_points > 0 && points.size() > 0) {
							new_points.push_back(pop_front(points));
							max_points--;
						}

						if (new_points.size() > 0)
						{
							if (!boost::iequals(new_points[0], _points[0])
								|| (new_points.size() > 1 && !boost::iequals(new_points[1], _points[1])))
							{
								departures[callsign] = new_points;
								renderDepartures = true;
							}
						}
					}


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

void refresh_ctrl_list()
{
	clear_ctrl_list();
	controller_list_box->clearLines();
	qlc_list_box->clearLines();
	for (int i = 0; i < 9; i++)
	{
		POSITIONS pos = static_cast<POSITIONS>(i);
		for (auto& s : controller_map)
		{
			Controller& c = *s.second;
			std::string callsign = c.getCallsign();

			if (dist(USER->getLatitude(), USER->getLongitude(), c.getLatitude(), c.getLongitude()) <= USER->getVisibility())
			{
				switch (pos)
				{
				case POSITIONS::OBSERVER:
				{
					if (!obs_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::OBSERVER)
					{
						std::vector<std::string> data;

						data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
						data.push_back("1A");
						data.push_back(frequency_to_string(c.userdata.frequency[0]));
						data.push_back(std::to_string(c.getIdentity()->controller_rating));

						add_to_ctrl_list(callsign, data, obs_list);
						add_to_qlctrl_list(callsign, data, ql_obs_list);

					}
					break;
				}
				case POSITIONS::DELIVERY:
				{
					if (!del_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::DELIVERY)
					{
						std::vector<std::string> data;

						data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
						data.push_back("1A");
						data.push_back(frequency_to_string(c.userdata.frequency[0]));
						data.push_back(std::to_string(c.getIdentity()->controller_rating));

						add_to_ctrl_list(callsign, data, del_list);
						add_to_qlctrl_list(callsign, data, ql_del_list);

					}
					break;
				}
				case POSITIONS::GROUND:
				{
					if (!gnd_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::GROUND)
					{
						std::vector<std::string> data;

						data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
						data.push_back("1A");
						data.push_back(frequency_to_string(c.userdata.frequency[0]));
						data.push_back(std::to_string(c.getIdentity()->controller_rating));

						add_to_ctrl_list(callsign, data, gnd_list);
						add_to_qlctrl_list(callsign, data, ql_gnd_list);

					}
					break;
				}
				case POSITIONS::TOWER:
				{
					if (!twr_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::TOWER)
					{
						std::vector<std::string> data;

						data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
						data.push_back("1A");
						data.push_back(frequency_to_string(c.userdata.frequency[0]));
						data.push_back(std::to_string(c.getIdentity()->controller_rating));

						add_to_ctrl_list(callsign, data, twr_list);
						add_to_qlctrl_list(callsign, data, ql_twr_list);

					}
					break;
				}
				case POSITIONS::DEPARTURE:
				{
					if (!dep_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::DEPARTURE)
					{
						std::vector<std::string> data;

						data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
						data.push_back("1A");
						data.push_back(frequency_to_string(c.userdata.frequency[0]));
						data.push_back(std::to_string(c.getIdentity()->controller_rating));

						add_to_ctrl_list(callsign, data, dep_list);
						add_to_qlctrl_list(callsign, data, ql_dep_list);

					}
					break;
				}
				}
			}
		}

		switch (pos)
		{
		case POSITIONS::OBSERVER:
		{
			if (!obs_list.empty())
				controller_list_box->addLineTop("------------OBSERVER----------", CHAT_TYPE::MAIN);
			if (!ql_obs_list.empty())
				qlc_list_box->addLineTop("-----OBS-----", CHAT_TYPE::MAIN);
			break;
		}
		case POSITIONS::DELIVERY:
		{
			if (!del_list.empty())
				controller_list_box->addLineTop("------------DELIVERY----------", CHAT_TYPE::MAIN);
			if (!ql_del_list.empty())
				qlc_list_box->addLineTop("-----DEL-----", CHAT_TYPE::MAIN);
			break;
		}
		case POSITIONS::GROUND:
		{
			if (!gnd_list.empty())
				controller_list_box->addLineTop("-------------GROUND-----------", CHAT_TYPE::MAIN);
			if (!ql_gnd_list.empty())
				qlc_list_box->addLineTop("-----GND-----", CHAT_TYPE::MAIN);
			break;
		}
		case POSITIONS::TOWER:
		{
			if (!twr_list.empty())
				controller_list_box->addLineTop("-------------TOWER------------", CHAT_TYPE::MAIN);
			if (!ql_twr_list.empty())
				qlc_list_box->addLineTop("-----TWR-----", CHAT_TYPE::MAIN);
			break;
		}
		case POSITIONS::DEPARTURE:
		{
			if (!dep_list.empty())
				controller_list_box->addLineTop("-----------DEPARTURE----------", CHAT_TYPE::MAIN);
			if (!ql_dep_list.empty())
				qlc_list_box->addLineTop("-----DEP-----", CHAT_TYPE::MAIN);
			break;
		}
		}
	}

	renderDrawings = true;
}

void add_to_ctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, ChatLine*>& store)
{
	ChatLine* c = new ChatLine("", CHAT_TYPE::MAIN, controller_list_box);
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
	controller_list_box->gen_points();

	store.emplace(callsign, c);
}

void add_to_qlctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, ChatLine*>& store)
{
	ChatLine* c = new ChatLine("", CHAT_TYPE::MAIN, qlc_list_box);
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
	qlc_list_box->gen_points();

	store.emplace(callsign, c);
}

/*void check_del_ctrl_list(Controller& controller)
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
}*/

/*void refresh_ctrl_list()
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
}*/

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
	clear_ctrl_list(gnd_list);
	clear_ctrl_list(twr_list);
	clear_ctrl_list(dep_list);
	clear_qlctrl_list(ql_obs_list);
	clear_qlctrl_list(ql_del_list);
	clear_qlctrl_list(ql_gnd_list);
	clear_qlctrl_list(ql_twr_list);
	clear_qlctrl_list(ql_dep_list);
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
