#include "interfaces.h"

InterfaceFrame* controller_list = nullptr, * main_chat = nullptr;

void RenderControllerList(bool open, double x_, double y_)
{
	controller_list = new InterfaceFrame(CONTROLLER_INTERFACE);

	controller_list->title = "CONTROLLERS";

	int width = 210, x = x_ == -1 ? (CLIENT_WIDTH / 2) - (width / 2) : x_;
	int height = 300, y = y_ == -1 ? (CLIENT_HEIGHT / 2) - (height / 2) : y_;
	controller_list->Pane1(x, width, y, height);
	double spacing_x = 0.15, spacing_y = 0.07;
	double start_x = 1.135, start_y = 0.10;

	ClickButton* alpha = new ClickButton(controller_list, "ALPHA Sort", x + (width - (width * (start_x - spacing_x))),
		90.0, y + (height - (height * (start_y + spacing_y))), 25.0);
	controller_list->children[alpha->index = CONTROLLER_LIST_ALPHA] = alpha;
	ClickButton* pofButton = new ClickButton(controller_list, "POF Only", x + (width - (width * (start_x - spacing_x))) + 105.0,
		90.0, y + (height - (height * (start_y += spacing_y))), 25.0);
	controller_list->children[pofButton->index = CONTROLLER_LIST_POF] = pofButton;

	start_x = 1.15;

	std::vector<ChatLine*> list;
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list.push_back(new ChatLine("", CHAT_TYPE::MAIN));

	double route_box_size = 150.0;
	controller_list_box = new DisplayBox(controller_list, list, list.size(), x + (width - (width * (start_x - spacing_x))),
		width - 25, 5, y + (height - (height * (start_y += spacing_y))) - (route_box_size - 10), route_box_size, 5, false);
	controller_list->children[controller_list_box->index = CONTROLLER_LIST_BOX] = controller_list_box;


	spacing_y = 0.02;
	std::vector<ChatLine*> list2;
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
	double route_box_size2 = 75.0;
	controller_info_box = new DisplayBox(controller_list, list2, list2.size(), x + (width - (width * (start_x - spacing_x))),
		width - 25, 5, y + ((height - (height * (start_y += spacing_y))) - (route_box_size2 - 10)) - route_box_size, route_box_size2, 5, false);
	controller_list->children[controller_info_box->index = CONTROLLER_LIST_INFOBOX] = controller_info_box;

	CloseButton* closeb = new CloseButton(controller_list, 15, 15);
	controller_list->children[closeb->index = CONTROLLER_LIST_CLOSE] = closeb;

	if (open)
	{
		controller_list->doOpen(true, true);//delete's old object while opening, thisshould be before setting frame vector
	}
	else
	{
		controller_list->doClose();
	}

	controller_list->doInsert();
}

void RenderConnect(double x_, double y_)
{
	connectFrame = new InterfaceFrame(CONNECT_INTERFACE);
	connectFrame->title = "CONNECT";
	int width = 300, x = x_ == -1 ? (CLIENT_WIDTH / 2) - (width / 2) : x_;
	int height = 200, y = y_ == -1 ? (CLIENT_HEIGHT / 2) - (height / 2) : y_;
	connectFrame->Pane1(x, width, y, height);
	callsign_label = new Label(connectFrame, "Callsign:", x, 120.0, 10.0, y + (height - 30.0), 20, -10.0);
	connectFrame->children[callsign_label->index = CONN_CALLSIGN_LABEL] = callsign_label;
	name_label = new Label(connectFrame, "Full Name:", (x + 150.0), 120.0, 10.0, y + (height - 30.0), 20, -10.0);
	connectFrame->children[name_label->index = FULLNAME_LABEL] = name_label;
	user_label = new Label(connectFrame, "Username:", x, 120.0, 10.0, y + (height - 75.0), 20, -10.0);
	connectFrame->children[user_label->index = USERNAME_LABEL] = user_label;
	pass_label = new Label(connectFrame, "Password:", (x + 150.0), 120.0, 10.0, y + (height - 75.0), 20, -10.0);
	connectFrame->children[pass_label->index = PASSWORD_LABEL] = pass_label;
	connect_callsign = new InputField(connectFrame, x, 120.0, 10.0, y + (height - 50.0), 20, -10.0);
	connect_callsign->max_chars = 10;
	connectFrame->children[connect_callsign->index = CALLSIGN_INPUT] = connect_callsign;
	connect_fullname = new InputField(connectFrame, (x + 150.0), 120.0, 10.0, y + (height - 50.0), 20, -10.0);
	connectFrame->children[connect_fullname->index = FULLNAME_INPUT] = connect_fullname;
	connect_username = new InputField(connectFrame, x, 120.0, 10.0, y + (height - 95.0), 20, -10.0);
	connectFrame->children[connect_username->index = USERNAME_INPUT] = connect_username;
	connect_password = new InputField(connectFrame, (x + 150.0), 120.0, 10.0, y + (height - 95.0), 20, -10.0);
	connect_password->p_protected = true;
	connectFrame->children[connect_password->index = PASSWORD_INPUT] = connect_password;

	std::vector<std::string> options1;
	options1.push_back("Observer");
	options1.push_back("Student 1");
	options1.push_back("Student 2");
	options1.push_back("Student 3");
	options1.push_back("Controller 1");
	options1.push_back("Controller 2");
	options1.push_back("Controller 3");
	options1.push_back("Instructor 1");
	options1.push_back("Instructor 2");
	options1.push_back("Instructor 3");
	options1.push_back("Supervisor");
	options1.push_back("Administrator");
	connect_rating = new ComboBox(connectFrame, options1, (x + 15.0), -1, 10.0, y + (height - 130), 22.0, -10.0);
	connectFrame->children[connect_rating->index = RATING_COMBO] = connect_rating;
	std::vector<std::string> options2;
	options2.push_back("Observer");
	options2.push_back("Delivery");
	options2.push_back("Ground");
	options2.push_back("Tower");
	options2.push_back("Departure");
	options2.push_back("Approach");
	options2.push_back("Center");
	options2.push_back("Oceanic");
	options2.push_back("FSS");
	connect_position = new ComboBox(connectFrame, options2, (x + 15) + 163, -1, 10.0, y + (height - 130), 22.0, -10.0);
	connectFrame->children[connect_position->index = CONN_POSITION_COMBO] = connect_position;
	connect_closeb = new CloseButton(connectFrame, 15, 15);
	connectFrame->children[connect_closeb->index = CONN_CLOSE_BUTTON] = connect_closeb;
	ClickButton* okButton = new ClickButton(connectFrame, "CONNECT", x + 30.0, 100.0, y + 10.0 + (height - 190.0), 25.0);
	connectFrame->children[okButton->index = CONN_OKAY_BUTTON] = okButton;
	ClickButton* cancelButton = new ClickButton(connectFrame, "CANCEL", (x + 30.0) + 120.0, 100.0, y + 10.0 + (height - 190.0), 25.0);
	connectFrame->children[cancelButton->index = CONN_CANCEL_BUTTON] = cancelButton;

#ifdef _DEBUG
	connect_callsign->setInput("SM_SUP");
	connect_fullname->setInput("Samuel Mason");
	connect_username->setInput("971202");
	connect_password->setInput("583562");
	connect_rating->pos = 10;
#endif

	connectFrame->doOpen(false, true);//delete's old object while opening, this should be before setting vector

	connectFrame->doInsert();
}

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, int id) {
	InterfaceFrame* pm_frame = frames_def[id];
	if (!pm_frame || refresh) {

		pm_frame = new InterfaceFrame(id);
		pm_frame->title = "PRIVATE CHAT: " + callsign;
		int width = 400, x = x_ == -1 ? (CLIENT_WIDTH / 2) - (width / 2) : x_;
		int height = 155, y = y_ == -1 ? (CLIENT_HEIGHT / 2) - (height / 2) : y_;
		pm_frame->Pane1(x, width, y, height);
		const double spacing = 0.15, spacing_y = 0.13;
		double start_x = 1.15, start_y = 0.15;

		std::vector<ChatLine*> list;
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine(callsign + ": Hello There!", CHAT_TYPE::MAIN));
		double route_box_size = 90.0;
		DisplayBox* displayBox = new DisplayBox(pm_frame, list, 7, x + (width - (width * (start_x - spacing))), width * 0.93, 5, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		displayBox->prune_top = true;
		pm_frame->children[displayBox->index = PRIVATE_MESSAGE_BOX] = displayBox;

		InputField* pm_input = new InputField(pm_frame, x + (width - (width * (start_x - spacing))), width * 0.93, 5, (y - (route_box_size - 10)) + (height - (height * (start_y += spacing_y))), 20.0, 0.0);
		pm_frame->children[pm_input->index = PRIVATE_MESSAGE_INPUT] = pm_input;

		CloseButton* pm_closeb = new CloseButton(pm_frame, 15, 15);
		pm_frame->children[pm_closeb->index = PM_CLOSE_BOX] = pm_closeb;

		pm_frame->doOpen(true, true);//delete's old object while opening, thisshould be before setting frame vector

		pm_frame->doInsert();
	}
	else {
		if (!pm_frame->render) {
			pm_frame->doOpen(false, true);
		}
	}
}

void LoadMainChatInterface(bool refresh) {

	//set main pane
	const int controller_list_width = 95;
	const int arrow_offset = 15;//arrows that come with display box

	int x = (CLIENT_WIDTH / 3);
	double width = 0.66666666667;
	const int c_padding = 10, m_padding = 10;

	// 2 arrow offset's for both display boxes
	// if you want to make input box longer, remove an arrow offset
	const int width_offset = (controller_list_width + (arrow_offset + arrow_offset) + c_padding + m_padding);

	if (refresh) {
		if (main_chat) {
			main_chat->UpdatePane1(x, width, 0, 125);
			for (ChildFrame* child : main_chat->children) {
				if (child) {
					switch (child->index) {
						case MAIN_CONTROLLERS_BOX:
						{
							((DisplayBox*)child)->updatePos(x, controller_list_width, 5, 114);
						}
						break;
						case MAIN_CHAT_MESSAGES:
						{
							((DisplayBox*)child)->updatePos(x + (controller_list_width + arrow_offset),
								(CLIENT_WIDTH * width) - width_offset, 27, 87);
						}
						break;
						case MAIN_CHAT_INPUT:
						{
							((InputField*)child)->updatePos(x + (controller_list_width + arrow_offset),
								(CLIENT_WIDTH * width) - width_offset, 5, 20);
						}
						break;
					}
				}
			}
		}
	}
	else
	{
		main_chat = new InterfaceFrame(MAIN_CHAT_INTERFACE);

		main_chat->Pane1(x, width, 0, 125);

		//controller list
		std::vector<ChatLine*> list;
		//---Delivery---
		//1A - MIA_DEL
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		qlc_list_box = new DisplayBox(main_chat, list, 10, x, controller_list_width, 5, 5, 114, 5, true);
		main_chat->children[qlc_list_box->index = MAIN_CONTROLLERS_BOX] = qlc_list_box;

		//chatbox
		std::vector<ChatLine*> list2;
		list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list2.push_back(new ChatLine("", CHAT_TYPE::MAIN));
		list2.push_back(new ChatLine("[00:00:00] Performing Version Check..", CHAT_TYPE::SYSTEM));
		ChatLine* c = new ChatLine("[ERROR: Unable to retrieve version]", CHAT_TYPE::ERRORS);
		list2.push_back(c);
		c->playChatSound();

		main_chat_box = new DisplayBox(main_chat, list2, 6, x + (controller_list_width + arrow_offset),
			(CLIENT_WIDTH * width) - width_offset, m_padding, 27, 87, 10, false);
		main_chat_box->prune_top = true;
		main_chat->children[main_chat_box->index = MAIN_CHAT_MESSAGES] = main_chat_box;

		main_chat_input = new InputField(main_chat, x + (controller_list_width + arrow_offset),
			(CLIENT_WIDTH * width) - width_offset, 10.0, 5, 20, 5);
		main_chat->children[main_chat_input->index = MAIN_CHAT_INPUT] = main_chat_input;

		main_chat->doInsert();
	}
}

void sendSystemMessage(std::string message)
{
	main_chat_box->resetReaderIdx();
	ChatLine* c = new ChatLine(std::string("[00:00:00] ") + message, CHAT_TYPE::SYSTEM);
	main_chat_box->addLine(c);
	c->playChatSound();
	renderDrawings = true;
}

void sendErrorMessage(std::string message) {
	main_chat_box->resetReaderIdx();
	ChatLine* c = new ChatLine(std::string("[ERROR: ") + message + std::string("]"), CHAT_TYPE::ERRORS);
	main_chat_box->addLine(c);
	c->playChatSound();
	renderDrawings = true;
}

void sendMainChatMessage(InputField* focusField)
{
	if (connected)
	{
		sendUserMessage(99998, focusField->input);
	}
	main_chat_box->resetReaderIdx();
	main_chat_box->addLine(USER->getIdentity()->callsign + std::string(": ") + focusField->input, CHAT_TYPE::MAIN);
	renderDrawings = true;
	focusField->clearInput();
	focusField->setCursor();
	renderInputTextFocus = true;
}

void sendATCMessage(std::string message) {
	for (size_t i = 0; i < userStorage1.size(); i++) {
		User* curUsr = userStorage1[i];
		if (curUsr != NULL && curUsr != USER) {
			//sendUserMessage(*curUsr, message);
		}
	}
	main_chat_box->resetReaderIdx();
	ChatLine* c = new ChatLine(USER->getIdentity()->callsign + std::string(": ") + message, CHAT_TYPE::ATC);
	main_chat_box->addLine(c);
	c->playChatSound();
	renderDrawings = true;
}
