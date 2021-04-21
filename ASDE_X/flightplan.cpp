#include "flightplan.h"

User* opened_fp = NULL;

std::unordered_map<std::string, std::vector<std::string>> departures;

void Load_Unknown_FlightPlan_Interface(double x, double y, char* call_sign, bool refresh)
{
	std::string values[13] = { call_sign, "Unknown", "R1", "C172/L" , "IFR", "KMIA", "MMUN", 
		"MYNN", "10000", "08R", "3306" , "SOME REALLY REALLY LONG LONG SOMETHING ROUTE THAT WE NEED TO GET IN TO TEST WHATS HAPPENING WITH AN EVEN LONGER ONE SO WE CAN TEST ALL THREE LINES" , ""};
	Load_FlightPlan_Interface(x, y, values, refresh);
	opened_fp = NULL;
}

void Load_Known_No_FlightPlan_Interface(double x, double y, User& user, bool refresh)
{
	Identity& id = *user.getIdentity();
	Aircraft& acf = *user.getAircraft();
	std::string values[13] = { id.callsign, id.login_name, "", "" , "IFR",
		"", "", "", "", "", "", "", "" };
	Load_FlightPlan_Interface(x, y, values, refresh);
	opened_fp = &user;
}

void Load_FlightPlan_Interface(double x, double y, User& user, bool refresh)
{
	Identity &id = *user.getIdentity();
	Aircraft &acf = *user.getAircraft();
	FlightPlan& fp = *user.getAircraft()->getFlightPlan();
	std::cout << id.login_name << std::endl;
	std::string values[13] = { id.callsign, id.login_name, "R1", "A321/L" , "IFR", 
		fp.departure, fp.arrival, fp.alternate, fp.cruise, fp.scratchPad, fp.squawkCode, fp.route, fp.remarks };
	Load_FlightPlan_Interface(x, y, values, refresh);
	opened_fp = &user;
}

void Load_FlightPlan_Interface(double x_, double y_, std::string* strings, bool refresh) {
	if (single_opened_frames) {
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
	InterfaceFrame* fp_frame = frames[FP_INTERFACE];
	if (!fp_frame || refresh) {
		if (fp_frame) {
			delete fp_frame;
		}
		frames[FP_INTERFACE] = fp_frame = new InterfaceFrame(FP_INTERFACE);
		fp_frame->title = call + " - FLIGHTPLAN (" + name + " " + pilot_rating + ")";
		int width = 450, x = x_ == -1 ? (CLIENT_WIDTH / 2) - (width / 2) : x_;
		int height = 220, y = y_ == -1 ?(CLIENT_HEIGHT / 2) - (height / 2) : y_;
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
		ac_type->setUneditable(ac_type_txt);
		fp_frame->children[ac_type->index = FP_ACTYPE_INPUT] = ac_type;
		Label* fr_label = new Label(fp_frame, "Flight Rules:", x + (width - (width * (start_x -= spacing_x))), label_width, 10.0, y + (height - (height * start_y)), 20.0, 0.0);
		fr_label->centered = 2;
		fp_frame->children[fr_label->index = FP_FLIGHTRULES_LABEL] = fr_label;		
		ComboBox* comboBox1 = new ComboBox(fp_frame, options1, x + (width - (width * (start_x -= spacing_x))) + 10, (input_width - 20.0), 10.0, y + (height - (height * start_y)), 20.0, 0.0);
		fp_frame->children[comboBox1->index = FP_FLIGHTRULES_INPUT] = comboBox1;
		
		ClickButton* amendButton = new ClickButton(fp_frame, "AMEND PLAN", x + ((width - buttons_offset) - button_width), button_width, y + (height - (height * start_y)), 20.0);// set button from the left
		fp_frame->children[amendButton->index = FP_AMMEND_PLAN] = amendButton;

		
		//2nd Line
		start_x = 1.0;
		Label* depart_label = new Label(fp_frame, "Depart:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		depart_label->centered = 2;
		fp_frame->children[depart_label->index = FP_DEPART_LABEL] = depart_label;
		InputField* depart_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		depart_input->setUneditable(depart);
		fp_frame->children[depart_input->index = FP_DEPART_INPUT] = depart_input;
		Label* arrive_label = new Label(fp_frame, "Arrive:", x + (width - (width * (start_x -= spacing_x))), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		arrive_label->centered = 2;
		fp_frame->children[arrive_label->index = FP_ARRIVE_LABEL] = arrive_label;
		InputField* arrive_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		arrive_input->setUneditable(arrive);
		fp_frame->children[arrive_input->index = FP_ARRIVE_INPUT] = arrive_input;
		Label* altern_label = new Label(fp_frame, "Alternate:", x + (width - (width * (start_x -= spacing_x))), label_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		altern_label->centered = 2;
		fp_frame->children[altern_label->index = FP_ALTERNATE_LABEL] = altern_label;
		InputField* altern_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		altern_input->setUneditable(alternate);
		fp_frame->children[altern_input->index = FP_ALTERNATE_INPUT] = altern_input;

		ClickButton* refreshButton = new ClickButton(fp_frame, "REFRESH PLAN", x + ((width - buttons_offset) - button_width), button_width, y + (height - (height * (start_y += spacing_y))), 20.0);// set button from the left
		fp_frame->children[refreshButton->index = FP_REFRESH_PLAN] = refreshButton;

		//3rd Line
		start_x = 1.0;
		Label* cruise_label = new Label(fp_frame, "Cruise:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		cruise_label->centered = 2;
		fp_frame->children[cruise_label->index = FP_CRUISE_LABEL] = cruise_label;
		InputField* cruise_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		cruise_input->setUneditable(cruise);
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
		InputField* squawk_input = new InputField(fp_frame, x + (width - (width * (start_x -= spacing_x))), input_width, 10.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		squawk_input->setUneditable(a_squawk);
		fp_frame->children[squawk_input->index = FP_SQUAWK_INPUT] = squawk_input;

		ClickButton* assignButton = new ClickButton(fp_frame, "ASSIGN SQUAWK", x + ((width - buttons_offset) - button_width), button_width, y + (height - (height * (start_y += spacing_y))), 20.0);// set button from the left
		fp_frame->children[assignButton->index = FP_ASSIGN_SQUAWK] = assignButton;

		//4th Line
		start_x = 1.0;
		Label* route_label = new Label(fp_frame, "Route:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		route_label->centered = 2;
		fp_frame->children[route_label->index = FP_ROUTE_LABEL] = route_label;
		std::vector<ChatLine*> list;
		list.push_back(new ChatLine(route, CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		double route_box_size = 50.0;
		DisplayBox* displayBox = new DisplayBox(fp_frame, list, 3, x + (width - (width * (start_x -= spacing_x))), width * 0.815, 0.0, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		displayBox->editable = true;
		fp_frame->children[displayBox->index = FP_ROUTE_BOX] = displayBox;

		//5th Line
		start_x = 1.0;
		start_y += 0.12;
		Label* remarks_label = new Label(fp_frame, "Remarks:", x + (width - (width * start_x)), label_width, 0.0, y + (height - (height * (start_y + spacing_y))), 20.0, 0.0);
		remarks_label->centered = 2;
		fp_frame->children[remarks_label->index = FP_REMARKS_LABEL] = remarks_label;
		std::vector<ChatLine*> remarks_list;
		remarks_list.push_back(new ChatLine(remarks, CHAT_TYPE::MAIN));
		remarks_list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		route_box_size = 30.0;
		DisplayBox* remarks_box = new DisplayBox(fp_frame, remarks_list, 2, x + (width - (width * (start_x -= spacing_x))), width * 0.815, 0.0, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		remarks_box->editable = true;
		fp_frame->children[remarks_box->index = FP_REMARKS_BOX] = remarks_box;

		CloseButton* fp_closeb = new CloseButton(fp_frame, 15, 15);
		fp_frame->children[fp_closeb->index = FP_CLOSE_BUTTON] = fp_closeb;


		fp_frame->doOpen(false, true);
	} else {
		if (!fp_frame->render) {
			fp_frame->doOpen(false, true);
		}
	}
}