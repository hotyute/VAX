#include "calc_cycles.h"
#include "controller.h"
#include "flightplan.h"
#include "packets.h"

void add_to_ctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store);

void append_to_ctlr_List(std::string callsign, Controller& c, std::unordered_map<std::string,
	std::shared_ptr<ChatLine>>&list, std::unordered_map<std::string, std::shared_ptr<ChatLine>>& ql_list);

void add_to_qlctrl_list(std::string callsign, std::vector<std::string>& data, std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store);

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
	//TODO We shouldn't be using Collision logic when aircraft are stopped.
	if (!acf_map.empty()) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++)
		{
			Aircraft* acf1 = iter->second;
			if (acf1) {
				Aircraft& aircraft1 = *acf1;

				for (const auto& iter2 : acf_map)
				{

					Aircraft* acf2 = iter2.second;
					if (acf2 && acf2 != acf1) {
						Aircraft& aircraft2 = *acf2;
						bool notColliding = aircraft1.collisions.find(acf2) == aircraft1.collisions.end() &&
							aircraft2.collisions.find(acf1) == aircraft2.collisions.end();
						if (notColliding)
						{
							if (areColliding(acf1, acf2, 30.0)) {
								auto* collision = new Collision(acf1, acf2);
								collision->setUpdateFlag(COL_COLLISION_LINE, true);
								aircraft1.collisions.emplace(acf2, collision);
								aircraft2.collisions.emplace(acf1, collision);
								addCollisionToMirrors(collision);
								Collision_Map.emplace(acf1, collision);
								Collision_Map.emplace(acf2, collision);
							}
						}
						else
						{
							if (!futureDistanceCollide(acf1, acf2, 30.0)) {
								Collision* collision = aircraft1.collisions[acf2];
								Collision* collision2 = aircraft2.collisions[acf1];
								if (collision = collision2)
								{
									Collision* map_col = Collision_Map[acf1];
									Collision* map_col2 = Collision_Map[acf2];
									if (map_col == map_col2)
									{
										aircraft1.collisions.erase(acf2);
										aircraft2.collisions.erase(acf1);
										removeCollisionFromMirrors(collision);
										collision->setUpdateFlag(COL_COLLISION_LINE, true);
										//add collision to a queue to be deleted.
									}
								}
								printf("No Longer Colliding.\n");
							}
						}
					}
				}

				if (!aircraft1.collisions.empty() && !aircraft1.isCollision())
				{
					aircraft1.setUpdateFlag(ACF_COLLISION, true);
					aircraft1.setCollision(true);
				}
				else if (aircraft1.collisions.empty() && aircraft1.isCollision())
				{
					aircraft1.setUpdateFlag(ACF_COLLISION, true);
					aircraft1.setCollision(false);
				}
			}
		}
	}

	if (!Collision_Map.empty()) {
		for (const auto& iter : Collision_Map)
		{
			// iterator->first = key
			Collision* col = iter.second;
		}
	}
}

void CalcDepartures() {
	for (const auto& iter : acf_map)
	{
		Aircraft* acf1 = iter.second;
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
					rendererFlags["renderDepartures"] = true;
				}
			}
			else
			{

				if (!boost::iequals(fp.departure, icao) || icao.empty())
				{
				}
				else
				{
					//refresh
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
							rendererFlags["renderDepartures"] = true;
						}
					}
				}


				//check for removal
			}
		}
	}
}

void CalcControllerList() {
	if (!controller_map.empty())
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
		auto pos = static_cast<POSITIONS>(i);
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
						append_to_ctlr_List(callsign, c, obs_list, ql_obs_list);
					}
					break;
				}
				case POSITIONS::DELIVERY:
				{
					if (!del_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::DELIVERY)
					{
						append_to_ctlr_List(callsign, c, del_list, ql_del_list);
					}
					break;
				}
				case POSITIONS::GROUND:
				{
					if (!gnd_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::GROUND)
					{
						append_to_ctlr_List(callsign, c, gnd_list, ql_gnd_list);
					}
					break;
				}
				case POSITIONS::TOWER:
				{
					if (!twr_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::TOWER)
					{
						append_to_ctlr_List(callsign, c, twr_list, ql_twr_list);
					}
					break;
				}
				case POSITIONS::DEPARTURE:
				{
					if (!dep_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::DEPARTURE)
					{
						append_to_ctlr_List(callsign, c, dep_list, ql_dep_list);
					}
					break;
				}
				case POSITIONS::APPROACH:
				{
					if (!app_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::APPROACH)
					{
						append_to_ctlr_List(callsign, c, app_list, ql_app_list);
					}
					break;
				}
				case POSITIONS::CENTER:
				{
					if (!ctr_list.count(callsign) && c.getIdentity()->controller_position == POSITIONS::CENTER)
					{
						append_to_ctlr_List(callsign, c, ctr_list, ql_ctr_list);
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
		case POSITIONS::APPROACH:
		{
			if (!app_list.empty())
				controller_list_box->addLineTop("-----------APPROACH-----------", CHAT_TYPE::MAIN);
			if (!ql_app_list.empty())
				qlc_list_box->addLineTop("-----APP-----", CHAT_TYPE::MAIN);
			break;
		}
		case POSITIONS::CENTER:
		{
			if (!ctr_list.empty())
				controller_list_box->addLineTop("------------CENTER------------", CHAT_TYPE::MAIN);
			if (!ql_ctr_list.empty())
				qlc_list_box->addLineTop("-----CTR-----", CHAT_TYPE::MAIN);
			break;
		}
		}
	}

	rendererFlags["drawings"] = true;
}

void add_to_ctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
	auto c = std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box);
	std::string controller_user;
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
	controller_list_box->consolidate_lines();
	controller_list_box->gen_points();

	store.emplace(callsign, c);
}

void add_to_qlctrl_list(std::string callsign, std::vector<std::string>& data,
	std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
	auto c = std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box);
	std::string controller_user;
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
	qlc_list_box->consolidate_lines();
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

void append_to_ctlr_List(std::string callsign, Controller& c, std::unordered_map<std::string, std::shared_ptr<ChatLine>>& list,
	std::unordered_map<std::string, std::shared_ptr<ChatLine>>& ql_list) {
	std::vector<std::string> data;

	data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
	data.push_back("1A");
	data.push_back(frequency_to_string(c.userdata.frequency[0]));
	data.push_back(std::to_string(c.getIdentity()->controller_rating));

	add_to_ctrl_list(callsign, data, list);
	add_to_qlctrl_list(callsign, data, ql_list);
}

void clear_ctrl_list(std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
	auto it = store.begin();
	while (it != store.end())
	{
		remove_ctrl_list((*it).second);
		it = store.erase(it);
	}
}

void clear_qlctrl_list(std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
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
	clear_ctrl_list(app_list);
	clear_qlctrl_list(ql_obs_list);
	clear_qlctrl_list(ql_del_list);
	clear_qlctrl_list(ql_gnd_list);
	clear_qlctrl_list(ql_twr_list);
	clear_qlctrl_list(ql_dep_list);
	clear_qlctrl_list(ql_app_list);
}

void remove_ctrl_list(std::shared_ptr<ChatLine>& c)
{
	controller_list_box->removeLine(c);
	rendererFlags["drawings"] = true;
}

void remove_qlctrl_list(std::shared_ptr<ChatLine>& c)
{
	qlc_list_box->removeLine(c);
	rendererFlags["drawings"] = true;
}
