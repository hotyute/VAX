#include "calc_cycles.h"

DWORD __stdcall CalcThread1(LPVOID)
{
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	boost::posix_time::time_duration time;

	while (true)
	{
		start = boost::posix_time::microsec_clock::local_time();

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
			for (auto iter = controller_map.begin(); iter != controller_map.end(); iter++)
			{
				Controller* ctlr = iter->second;
				if (ctlr) 
				{
					std::string callsign = ctlr->getCallsign();

					if (dist(USER->getLatitude(), USER->getLongitude(), ctlr->getLatitude(), ctlr->getLongitude()) <= USER->getVisibility())
					{
						if (i == 0)
						{
							if (!obs_list.count(callsign) && ctlr->getIdentity()->controller_position == 0)
							{

								std::vector<std::string> data;

								data.push_back(std::to_string(ctlr->getIdentity()->controller_position));
								data.push_back("1A");
								data.push_back(frequency_to_string(ctlr->frequency[0]));
								data.push_back(std::to_string(ctlr->getIdentity()->controller_rating));

								add_to_ctrl_list(callsign, data, obs_list);

								if (obs_list.empty())
									controller_list_box->addLineTop("------------OBSERVER----------", CHAT_TYPE::MAIN);

								renderDrawings = true;

							}
						}
						else if (i == 1)
						{
							if (!del_list.count(callsign) && ctlr->getIdentity()->controller_position == 1)
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
					}
					else
					{
						//check for removal
					}
				}
			}
			//for (auto &iter : ctrl_list)
			//{

			//}
		}
	}
}

void add_to_ctrl_list(std::string& callsign, std::vector<std::string>& data, 
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

	std::cout << atodd(data[3]) << std::endl;

	if (atodd(data[3]) == 10 || atodd(data[3]) == 11)
		c->setType(CHAT_TYPE::SUP_POS);

	controller_list_box->addLineTop(c);

	store.emplace(callsign, c);
}
