#include "flightplan.h"

#include "constants.h"
#include "inputfield.h"

User* opened_fp = NULL;
int squawk_range = 1000;

std::unordered_map<std::string, std::vector<std::string>> departures;
std::unordered_map<std::string, ChatLine*> obs_list, ql_obs_list, del_list, ql_del_list, gnd_list, ql_gnd_list,
twr_list, ql_twr_list, dep_list, ql_dep_list;

void Load_Unknown_FlightPlan_Interface(double x, double y, char* call_sign, bool refresh)
{
	std::string values[13] = { call_sign, "Unknown", PILOT_RATINGS[0], "C172/L" , "IFR", "KMIA", "MMUN",
		"MYNN", "10000", "08R", "3306" , "SOME REALLY REALLY LONG LONG SOMETHING ROUTE THAT WE NEED TO GET IN TO TEST WHATS HAPPENING WITH AN EVEN LONGER ONE SO WE CAN TEST ALL THREE LINES" , "" };
	Load_FlightPlan_Interface(x, y, values, refresh);
	opened_fp = NULL;
}

void Load_Known_No_FlightPlan_Interface(double x, double y, Aircraft& user, bool refresh)
{
	Identity& id = *user.getIdentity();
	std::string values[13] = { id.callsign, id.login_name, "", "" , "IFR",
		"", "", "", "", "", "", "", "" };
	Load_FlightPlan_Interface(x, y, values, refresh);
	opened_fp = &user;
}

void Load_FlightPlan_Interface(double x, double y, Aircraft& user, bool refresh)
{
	Identity& id = *user.getIdentity();
	FlightPlan& fp = *user.getFlightPlan();
#ifdef _DEBUG
	std::cout << id.login_name << std::endl;
#endif
	std::string values[13] = { id.callsign, id.login_name, PILOT_RATINGS[id.pilot_rating], fp.acType , "IFR",
		fp.departure, fp.arrival, fp.alternate, fp.cruise, fp.scratchPad, fp.squawkCode, fp.route, fp.remarks };
	Load_FlightPlan_Interface(x, y, values, refresh);
	opened_fp = &user;
}

void Load_FlightPlan_Interface(double x_, double y_, std::string* strings, bool refresh) {
	if (single_opened_frames && !opened_fp) {
		sendErrorMessage("This is not allowed (Designated Interface)");
		return;
	}
	std::vector<std::string> options1;
	options1.push_back("IFR");
	options1.push_back("VFR");
	options1.push_back("SVFR");
	std::string call = strings[0], name = strings[1], pilot_rating = strings[2],
		ac_type_txt = strings[3], fr_text = strings[4], depart = strings[5],
		arrive = strings[6], alternate = strings[7], cruise = strings[8],
		scratch = strings[9], a_squawk = strings[10], route = strings[11],
		remarks = strings[12];
	if (!fp_frame || refresh) {
		fp_frame = new InterfaceFrame(FP_INTERFACE);
		fp_frame->title = call + " - FLIGHTPLAN (" + name + " " + pilot_rating + ")";
		int width = 450, x = x_ == -1 ? (CLIENT_WIDTH / 2) - (width / 2) : x_;
		int height = 220, y = y_ == -1 ? (CLIENT_HEIGHT / 2) - (height / 2) : y_;
		fp_frame->Pane1(x, width, y, height);
		const double spacing_x = 0.125, spacing_y = 0.13;
		double start_x = 1.0, start_y = 0.2;
		double input_width = 50.0, label_width = 55.0, button_width = 85.0, buttons_offset = (width * 0.035);
		//1st Line
		Label* callsign_label = new Label(fp_frame, "Callsign:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * start_y)), 20.0, 0.0);
		callsign_label->centered = 2;
		fp_frame->children[callsign_label->index = FP_CALLSIGN_LABEL] = callsign_label;
		InputField* call_sign = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * start_y)), 20.0, 0.0);
		call_sign->setUneditable(call);
		fp_frame->children[call_sign->index = FP_CALLSIGN_INPUT] = call_sign;
		Label* actype_label = new Label(fp_frame, "A/C Type:", x + (width - (width * (start_x -= spacing_x))), label_width, 0.0, y + (height - (height * start_y)), 20.0, 0.0);
		actype_label->centered = 2;
		fp_frame->children[actype_label->index = FP_ACTYPE_LABEL] = actype_label;
		InputField* ac_type = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * start_y)), 20.0, 0.0);
		ac_type->setInput(ac_type_txt);
		ac_type->max_chars = 8;
		ac_type->centered = 2;
		fp_frame->children[ac_type->index = FP_ACTYPE_INPUT] = ac_type;
		Label* fr_label = new Label(fp_frame, "Flight Rules:", x + (width - (width * (start_x -= spacing_x))), label_width, 10.0, y + (height - (height * start_y)), 20.0, 0.0);
		fr_label->centered = 2;
		fp_frame->children[fr_label->index = FP_FLIGHTRULES_LABEL] = fr_label;
		ComboBox* comboBox1 = new ComboBox(fp_frame, options1, x + (width - (width * (start_x -= spacing_x))) + 10, (input_width - 20.0), 10.0, y + (height - (height * start_y)), 20.0, 0.0);
		fp_frame->children[comboBox1->index = FP_FLIGHTRULES_INPUT] = comboBox1;

		ClickButton* amendButton = new ClickButton(fp_frame, "AMEND PLAN", x + ((width - buttons_offset) - button_width), button_width, y + (height - (height * start_y)) - 2, 24.0);// set button from the left
		amendButton->border_pix = 3;
		fp_frame->children[amendButton->index = FP_AMMEND_PLAN] = amendButton;


		//2nd Line
		start_x = 1.0;
		Label* depart_label = new Label(fp_frame, "Depart:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		depart_label->centered = 2;
		fp_frame->children[depart_label->index = FP_DEPART_LABEL] = depart_label;
		InputField* depart_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		depart_input->setInput(depart);
		depart_input->max_chars = 4;
		depart_input->centered = 2;
		fp_frame->children[depart_input->index = FP_DEPART_INPUT] = depart_input;
		Label* arrive_label = new Label(fp_frame, "Arrive:", x + (width - (width * (start_x -= spacing_x))), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		arrive_label->centered = 2;
		fp_frame->children[arrive_label->index = FP_ARRIVE_LABEL] = arrive_label;
		InputField* arrive_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		arrive_input->setInput(arrive);
		arrive_input->max_chars = 4;
		arrive_input->centered = 2;
		fp_frame->children[arrive_input->index = FP_ARRIVE_INPUT] = arrive_input;
		Label* altern_label = new Label(fp_frame, "Alternate:", x + (width - (width * (start_x -= spacing_x))), label_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		altern_label->centered = 2;
		fp_frame->children[altern_label->index = FP_ALTERNATE_LABEL] = altern_label;
		InputField* altern_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		altern_input->setInput(alternate);
		altern_input->max_chars = 4;
		altern_input->centered = 2;
		fp_frame->children[altern_input->index = FP_ALTERNATE_INPUT] = altern_input;

		ClickButton* refreshButton = new ClickButton(fp_frame, "REFRESH PLAN", x + ((width - buttons_offset) - button_width), button_width, y + (height - (height * (start_y += spacing_y))) - 2, 24.0);// set button from the left
		refreshButton->border_pix = 3;
		fp_frame->children[refreshButton->index = FP_REFRESH_PLAN] = refreshButton;

		//3rd Line
		start_x = 1.0;
		Label* cruise_label = new Label(fp_frame, "Cruise:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		cruise_label->centered = 2;
		fp_frame->children[cruise_label->index = FP_CRUISE_LABEL] = cruise_label;
		InputField* cruise_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		cruise_input->numbers = true;
		cruise_input->max_chars = 5;
		cruise_input->centered = true;
		cruise_input->setInput(cruise);
		fp_frame->children[cruise_input->index = FP_CRUISE_INPUT] = cruise_input;
		Label* scratch_label = new Label(fp_frame, "Scratch:", x + (width - (width * (start_x -= spacing_x))), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		scratch_label->centered = 2;
		fp_frame->children[scratch_label->index = FP_SCRATCH_LABEL] = scratch_label;
		InputField* scratch_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		scratch_input->setUneditable(scratch);
		fp_frame->children[scratch_input->index = FP_SCRATCH_INPUT] = scratch_input;
		Label* squawk_label = new Label(fp_frame, "Squawk:", x + (width - (width * (start_x -= spacing_x))), label_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		squawk_label->centered = 2;
		fp_frame->children[squawk_label->index = FP_SQUAWK_LABEL] = squawk_label;
		squawk_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		squawk_input->setUneditable(a_squawk);
		squawk_input->numbers = true;
		fp_frame->children[squawk_input->index = FP_SQUAWK_INPUT] = squawk_input;

		ClickButton* assignButton = new ClickButton(fp_frame, "ASSIGN SQUAWK", x + ((width - buttons_offset) - button_width), button_width, y + (height - (height * (start_y += spacing_y))) - 2, 24.0);// set button from the left
		assignButton->border_pix = 3;
		fp_frame->children[assignButton->index = FP_ASSIGN_SQUAWK] = assignButton;

		//4th Line
		start_x = 1.0;
		Label* route_label = new Label(fp_frame, "Route:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		route_label->centered = 2;
		fp_frame->children[route_label->index = FP_ROUTE_LABEL] = route_label;
		double route_box_size = 50.0;
		DisplayBox* displayBox = new DisplayBox(fp_frame, x + (width - (width * (start_x -= spacing_x))), width * 0.815, 0.0, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		std::vector<ChatLine*> list;
		list.push_back(new ChatLine(route, CHAT_TYPE::MAIN, displayBox));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN, displayBox));
		displayBox->setList(list, 3);
		displayBox->editable = true;
		displayBox->caps = true;
		fp_frame->children[displayBox->index = FP_ROUTE_BOX] = displayBox;

		//5th Line
		start_x = 1.0;
		start_y += 0.12;
		Label* remarks_label = new Label(fp_frame, "Remarks:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		remarks_label->centered = 2;
		fp_frame->children[remarks_label->index = FP_REMARKS_LABEL] = remarks_label;

		route_box_size = 30.0;
		DisplayBox* remarks_box = new DisplayBox(fp_frame, x + (width - (width * (start_x -= spacing_x))), width * 0.815, 0.0, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		remarks_box->editable = true;
		remarks_box->caps = true;
		std::vector<ChatLine*> remarks_list;
		remarks_list.push_back(new ChatLine(remarks, CHAT_TYPE::MAIN, remarks_box));
		remarks_list.push_back(new ChatLine("", CHAT_TYPE::MAIN, remarks_box));
		remarks_box->setList(remarks_list, 2);
		fp_frame->children[remarks_box->index = FP_REMARKS_BOX] = remarks_box;

		CloseButton* fp_closeb = new CloseButton(fp_frame, 15, 15);
		fp_frame->children[fp_closeb->index = FP_CLOSE_BUTTON] = fp_closeb;


		fp_frame->doOpen(false, true);

		fp_frame->doInsert();
	}
	else {
		if (!fp_frame->render) {
			fp_frame->doOpen(false, true);
		}
	}
}

void PullFPData(Aircraft* user)
{
	if (user)
	{
		if (acf_map[user->getCallsign()] == user)
		{
			InputField* fp_depart = ((InputField*)fp_frame->children[FP_DEPART_INPUT]);
			InputField* fp_arrival = ((InputField*)fp_frame->children[FP_ARRIVE_INPUT]);
			InputField* fp_assigned_sq = ((InputField*)fp_frame->children[FP_SQUAWK_INPUT]);
			DisplayBox* fp_route = ((DisplayBox*)fp_frame->children[FP_ROUTE_BOX]);
			DisplayBox* fp_remarks = ((DisplayBox*)fp_frame->children[FP_REMARKS_BOX]);

			std::string depart, arrival, route, remarks, a_sq;
			depart = fp_depart->input.c_str();
			arrival = fp_arrival->input.c_str();
			a_sq = fp_assigned_sq->input.c_str();
			route = fp_route->getLinesComb();
			remarks = fp_remarks->getLinesComb();

			FlightPlan& fp = *user->getFlightPlan();
			bool update_required = false;
			if (fp.route != route || fp.remarks != remarks || fp.departure != depart
				|| fp.arrival != arrival || fp.squawkCode != a_sq)
			{
				update_required = true;
			}

			fp.departure = depart;
			fp.arrival = arrival;
			fp.route = route;
			fp.remarks = remarks;
			fp.squawkCode = a_sq;

			if (update_required)
			{
				fp.cycle++;
				if (connected)
				{
					sendFlightPlan(*user);
				}
			}
		}
	}
}