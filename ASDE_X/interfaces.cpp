#include "interfaces.h"
#include "radio.h"
#include "displaybox.h"
#include "comms.h"
#include "tools.h"

#include <boost/algorithm/string.hpp>
#include <regex>
#include <string>

#include "usermanager.h"

std::vector<std::string> pm_callsigns(20);

InterfaceFrame* controller_list = nullptr, * main_chat = nullptr, * terminal_cmd = nullptr, * communications = nullptr;

void RenderControllerList(bool open, double x_, double y_)
{
	controller_list = new InterfaceFrame(CONTROLLER_INTERFACE);

	controller_list->title = "CONTROLLERS";

	int width = 210, x = x_ == -1 ? (CLIENT_WIDTH / 2.0) - (width / 2.0) : x_;
	int height = 300, y = y_ == -1 ? (CLIENT_HEIGHT / 2.0) - (height / 2.0) : y_;
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

	double route_box_size = 150.0;
	controller_list_box = new DisplayBox(controller_list, x + (width - (width * (start_x - spacing_x))),
		width - 25, 5, y + (height - (height * (start_y += spacing_y))) - (route_box_size - 10), route_box_size, 5, false);
	std::vector<std::shared_ptr<ChatLine>> list;
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box));
	controller_list_box->setList(list, list.size());
	controller_list->children[controller_list_box->index = CONTROLLER_LIST_BOX] = controller_list_box;


	spacing_y = 0.02;
	double route_box_size2 = 75.0;
	controller_info_box = new DisplayBox(controller_list, x + (width - (width * (start_x - spacing_x))),
		width - 25, 5, y + ((height - (height * (start_y += spacing_y))) - (route_box_size2 - 10)) - route_box_size, route_box_size2, 5, false);
	std::vector<std::shared_ptr<ChatLine>> list2;
	list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_info_box));
	list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_info_box));
	list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_info_box));
	list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_info_box));
	list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_info_box));
	controller_info_box->setList(list2, list2.size());
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
	Identity& id = *USER->getIdentity();
	bool data_set = !boost::icontains(id.callsign, "NOT_LOGGED") && !id.callsign.empty();
	connectFrame = new InterfaceFrame(CONNECT_INTERFACE);
	connectFrame->title = "CONNECT";
	int width = 300, x = x_ == -1 ? (CLIENT_WIDTH / 2.0) - (width / 2.0) : x_;
	int height = 200, y = y_ == -1 ? (CLIENT_HEIGHT / 2.0) - (height / 2.0) : y_;
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
	connect_callsign->caps = true;
	connect_callsign->max_chars = 10;

	connectFrame->children[connect_callsign->index = CALLSIGN_INPUT] = connect_callsign;
	connect_fullname = new InputField(connectFrame, (x + 150.0), 120.0, 10.0, y + (height - 50.0), 20, -10.0);
	connectFrame->children[connect_fullname->index = FULLNAME_INPUT] = connect_fullname;
	connect_username = new InputField(connectFrame, x, 120.0, 10.0, y + (height - 95.0), 20, -10.0);
	connectFrame->children[connect_username->index = USERNAME_INPUT] = connect_username;
	connect_username->numbers = true;
	connect_username->max_chars = 20;
	connect_password = new InputField(connectFrame, (x + 150.0), 120.0, 10.0, y + (height - 95.0), 20, -10.0);
	connect_password->p_protected = true;
	connect_password->max_chars = 20;
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

	if (data_set)
	{
		connect_callsign->setInput(id.callsign);
		connect_fullname->setInput(id.login_name);
		connect_username->setInput(id.username);
		connect_password->setInput(id.password);
		connect_rating->pos = id.controller_rating;
		connect_position->pos = static_cast<int>(id.controller_position);
	}
	else
	{
#ifdef _DEBUG
		connect_callsign->setInput("SM_SUP");
		connect_fullname->setInput("Samuel Mason");
		connect_username->setInput("971202");
		connect_password->setInput("583562");
		connect_rating->pos = 10;
#endif
	}

	connectFrame->doOpen(false, true);//delete's old object while opening, this should be before setting vector

	connectFrame->doInsert();
}

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, bool open, int id) {
	InterfaceFrame* pm_frame = frames_def[id];
	if (!pm_frame || refresh)
	{

		pm_frame = new InterfaceFrame(id);
		pm_frame->title = "PRIVATE CHAT: " + callsign;
		pm_callsigns[id] = callsign;
		int width = 400, x = x_ == -1 ? (CLIENT_WIDTH / 2.0) - (width / 2.0) : x_;
		int height = 155, y = y_ == -1 ? (CLIENT_HEIGHT / 2.0) - (height / 2.0) : y_;
		pm_frame->Pane1(x, width, y, height);
		const double spacing = 0.15, spacing_y = 0.13;
		double start_x = 1.15, start_y = 0.15;

		double route_box_size = 90.0;
		DisplayBox* displayBox = new DisplayBox(pm_frame, x + (width - (width * (start_x - spacing))), width * 0.93, 5, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		std::vector<std::shared_ptr<ChatLine>> list;
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, displayBox));
		displayBox->setList(list, 7);
		displayBox->prune_top = true;
		pm_frame->children[displayBox->index = PRIVATE_MESSAGE_BOX] = displayBox;

		InputField* pm_input = new InputField(pm_frame, x + (width - (width * (start_x - spacing))), width * 0.93, 5, (y - (route_box_size - 10)) + (height - (height * (start_y += spacing_y))), 20.0, 0.0);
		pm_frame->children[pm_input->index = PRIVATE_MESSAGE_INPUT] = pm_input;

		CloseButton* pm_closeb = new CloseButton(pm_frame, 15, 15);
		pm_frame->children[pm_closeb->index = PM_CLOSE_BOX] = pm_closeb;

		if (open)
		{
			pm_frame->doOpen(true, true);//delete's old object while opening, thisshould be before setting frame vector
		}
		else
		{
			pm_frame->doClose();
		}

		pm_frame->doInsert();
	}
	else
	{
		if (!pm_frame->render)
		{
			pm_frame->doOpen(false, true);
		}
	}
}

void RenderTerminalCommands(bool refresh)
{
	if (focusChild == terminal_input)
	{
		terminal_input->removeFocus();
		main_chat_input->setFocus();
	}

	terminal_cmd = new InterfaceFrame(TERMINAL_COMMAND);
	terminal_cmd->title = "CONNECT";
	terminal_cmd->stripped = true;
	int x = 30, y = (CLIENT_HEIGHT - (CLIENT_HEIGHT / 6)) - 50;
	double width = 150.0, height = 30;
	terminal_cmd->Pane1(x, width, y, height);

	terminal_input = new InputField(terminal_cmd, x - 3, width, 0, y, height, 0);
	terminal_cmd->children[terminal_input->index = TERMINAL_INPUT] = terminal_input;
	terminal_input->show_border = false;
	terminal_input->rgb[0] = conf_clr[0];
	terminal_input->rgb[1] = conf_clr[1];
	terminal_input->rgb[2] = conf_clr[2];

	terminal_input->font = &confFont;
	terminal_input->base = &confBase;

	terminal_cmd->doOpen(true, false);

	terminal_cmd->doInsert();
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

	if (refresh)
	{
		if (main_chat)
		{
			main_chat->UpdatePane1(x, width, 0, 125);
			for (ChildFrame* child : main_chat->children)
			{
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

		qlc_list_box = new DisplayBox(main_chat, x, controller_list_width, 5, 5, 114, 5, true);
		std::vector<std::shared_ptr<ChatLine>> list;
		//---Delivery---
		//1A - MIA_DEL
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		list.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box));
		qlc_list_box->setList(list, 10);
		main_chat->children[qlc_list_box->index = MAIN_CONTROLLERS_BOX] = qlc_list_box;

		//chatbox
		main_chat_box = new DisplayBox(main_chat, x + (controller_list_width + arrow_offset),
			(CLIENT_WIDTH * width) - width_offset, m_padding, 27, 87, 10, false);
		main_chat_box->prune_top = true;
		std::vector<std::shared_ptr<ChatLine>> list2;
		list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, main_chat_box));
		list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, main_chat_box));
		list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, main_chat_box));
		list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, main_chat_box));
		list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, main_chat_box));
		list2.push_back(std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, main_chat_box));
		main_chat_box->setList(list2, 6);
		main_chat->children[main_chat_box->index = MAIN_CHAT_MESSAGES] = main_chat_box;

		sendSystemMessage("Performing Version Check..");
		sendErrorMessage("Unable to retrieve version");

		main_chat_input = new InputField(main_chat, x + (controller_list_width + arrow_offset),
			(CLIENT_WIDTH * width) - width_offset, 10.0, 5, 20, 5);
		main_chat->children[main_chat_input->index = MAIN_CHAT_INPUT] = main_chat_input;

		main_chat->doInsert();
	}
}

void sendSystemMessage(std::string message)
{
	main_chat_box->resetReaderIdx();
	const std::string* date1 = currentDateTime();
	std::string r = std::regex_replace(date1[1], std::regex("/"), ":");
	r.insert(2, ":");
	const auto c = std::make_shared<ChatLine>(std::string("[" + r + "] ") + 
	                                          message, CHAT_TYPE::SYSTEM, main_chat_box);
	main_chat_box->addLine(c);
	c->playChatSound();
	renderDrawings = true;
}

void sendErrorMessage(std::string message) {
	main_chat_box->resetReaderIdx();
	const auto c = std::make_shared<ChatLine>(std::string("[ERROR: ") + 
		message + std::string("]"), CHAT_TYPE::ERRORS, main_chat_box);
	main_chat_box->addLine(c);
	c->playChatSound();
	renderDrawings = true;
}

void sendMainChatMessage(InputField* focusField)
{
	std::string text = ASEL ? ASEL->getCallsign() + ", " + focusField->input : focusField->input;
	if (connected)
	{
		for (auto& it : COMMS_STORE)
		{
			if (it && !it->freq.empty() && it->tx->checked)
			{
				int freq = string_to_frequency(it->freq);
				if (freq != 99998)
					sendUserMessage(freq, ASEL ? ASEL->getCallsign() : "", text);
			}
		}
	}
	main_chat_box->resetReaderIdx();
	main_chat_box->addLine(USER->getIdentity()->callsign + std::string(": ") + text, CHAT_TYPE::MAIN);
	renderDrawings = true;
	focusField->clearInput();
	focusField->setCursor();
	focusField->render = true;
}

void sendPrivateChatMessage(InterfaceFrame& frame, InputField* focusField)
{
	if (connected)
	{
		std::string call_sign = pm_callsigns[frame.id];
		if (!call_sign.empty() && call_sign.find_first_not_of(' ') != std::string::npos)
			sendPrivateMessage(call_sign, focusField->input);
	}
	DisplayBox& box = *((DisplayBox*)frame.children[PRIVATE_MESSAGE_BOX]);
	box.resetReaderIdx();
	box.addLine(USER->getIdentity()->callsign + std::string(": ") + focusField->input, CHAT_TYPE::MAIN);
	renderDrawings = true;
	focusField->clearInput();
	focusField->setCursor();
	RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
}

void sendATCMessage(std::string message) {
	for (size_t i = 0; i < userStorage1.size(); i++) {
		User* curUsr = userStorage1[i];
		if (curUsr != NULL && curUsr != USER) {
			//sendUserMessage(*curUsr, message);
		}
	}
	main_chat_box->resetReaderIdx();
	const auto c = std::make_shared<ChatLine>(USER->getIdentity()->callsign + std::string(": ") + message, CHAT_TYPE::ATC, main_chat_box);
	main_chat_box->addLine(c);
	c->playChatSound();
	renderDrawings = true;
}
