#include "interfaces.h"

void RenderConnect(double x_, double y_)
{
	if (connectFrame) {
		delete connectFrame;
	}
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
	connectFrame->children[connect_callsign->index = CALLSIGN_INPUT] = connect_callsign;
	connect_fullname = new InputField(connectFrame, (x + 150.0), 120.0, 10.0, y + (height - 50.0), 20, -10.0);
	connectFrame->children[connect_fullname->index = FULLNAME_INPUT] = connect_fullname;
	connect_username = new InputField(connectFrame, x, 120.0, 10.0, y + (height - 95.0), 20, -10.0);
	connectFrame->children[connect_username->index = USERNAME_INPUT] = connect_username;
	connect_password = new InputField(connectFrame, (x + 150.0), 120.0, 10.0, y + (height - 95.0), 20, -10.0);
	connect_password->p_protected = true;
	connectFrame->children[connect_password->index = PASSWORD_INPUT] = connect_password;
	std::vector<std::string> options1;
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
	ComboBox* comboBox1 = new ComboBox(connectFrame, options1, (x + 15.0), 0.0, 10.0, y + (height - 130), 22.0, -10.0);
	connectFrame->children[comboBox1->index = RATING_COMBO] = comboBox1;
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
	ComboBox* comboBox2 = new ComboBox(connectFrame, options2, (x + 15) + 163, 0.0, 10.0, y + (height - 130), 22.0, -10.0);
	connectFrame->children[comboBox2->index = CONN_POSITION_COMBO] = comboBox2;
	connect_closeb = new CloseButton(connectFrame, 15, 15);
	connectFrame->children[connect_closeb->index = CONN_CLOSE_BUTTON] = connect_closeb;
	ClickButton* okButton = new ClickButton(connectFrame, "CONNECT", x + 30.0, 100.0, y + 10.0 + (height - 190.0), 25.0);
	connectFrame->children[okButton->index = CONN_OKAY_BUTTON] = okButton;
	ClickButton* cancelButton = new ClickButton(connectFrame, "CANCEL", (x + 30.0) + 120.0, 100.0, y + 10.0 + (height - 190.0), 25.0);
	connectFrame->children[cancelButton->index = CONN_CANCEL_BUTTON] = cancelButton;
	connectFrame->doOpen(false, true);
}

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, int index) {
	InterfaceFrame* pm_frame = frames[index];
	if (!pm_frame || refresh) {
		if (pm_frame) {
			delete pm_frame;
		}
		pm_frame = new InterfaceFrame(index);
		pm_frame->title = "PRIVATE CHAT " + callsign;
		int width = 400, x = x_ == -1 ? (CLIENT_WIDTH / 2) - (width / 2) : x_;
		int height = 155, y = y_ == -1 ? (CLIENT_HEIGHT / 2) - (height / 2) : y_;
		pm_frame->Pane1(x, width, y, height);
		const double spacing = 0.15, spacing_y = 0.13;
		double start_x = 1.15, start_y = 0.15;

		std::vector<ChatLine*> list;
		list.push_back(new ChatLine("Hello there!", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("1", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("2", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("3", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("4", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("5", CHAT_TYPE::MAIN));
		list.push_back(new ChatLine("6", CHAT_TYPE::MAIN));
		double route_box_size = 90.0;
		DisplayBox* displayBox = new DisplayBox(pm_frame, list, 7, x + (width - (width * (start_x - spacing))), width * 0.93, 5, y + (height - (height * (start_y += spacing_y)))
			- (route_box_size - 10), route_box_size, 5, false);
		pm_frame->children[displayBox->index = PRIVATE_MESSAGE_BOX] = displayBox;

		InputField* pm_input = new InputField(pm_frame, x + (width - (width * (start_x - spacing))), width * 0.93, 5, (y - (route_box_size - 10)) + (height - (height * (start_y += spacing_y))), 20.0, 0.0);
		pm_frame->children[pm_input->index = PRIVATE_MESSAGE_INPUT] = pm_input;

		CloseButton* pm_closeb = new CloseButton(pm_frame, 15, 15);
		pm_frame->children[pm_closeb->index = PM_CLOSE_BOX] = pm_closeb;


		pm_frame->doOpen(true, true);
	}
	else {
		if (!pm_frame->render) {
			pm_frame->doOpen(false, true);
		}
	}
}
