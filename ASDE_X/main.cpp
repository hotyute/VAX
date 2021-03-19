#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>

#include "projection.h"
#include "main.h"
#include "later.h"
#include "renderer.h"
#include "filereader.h"
#include "guicon.h"
#include "topbutton.h"
#include "dxfdrawing.h"
#include "tools.h"
#include "clinc2.h"
#include "usermanager.h"
#include "packets.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
HDC hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
PAINTSTRUCT ps;

/*  Make the class name into a global variable  */
TCHAR szClassName[] = TEXT("WindowsApp");
HWND text123, text124, lblNone, strUsers, lblNumUsrs;

bool done = false, connected = false;

const int proto_version = 32698;

InterfaceFrame* connectFrame = NULL;
InputField* connect_callsign = NULL, * connect_fullname = NULL, * connect_username = NULL, * connect_password = NULL, * textField = NULL;
Label* callsign_label = NULL, * name_label = NULL, * user_label = NULL, * pass_label = NULL;
CloseButton* connect_closeb = NULL;
DisplayBox* main_chat = NULL;

void handleConnect();
bool processCommands(std::string);

/* Components */

int WINAPI WinMain(HINSTANCE hThisInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int nFunsterStil)

{
	std::cout.precision(10);

	HWND hwnd;               /* This is the handle for our window */
	MSG messages;            /* Here messages to the application are saved */
	WNDCLASSEX wincl;        /* Data structure for the windowclass */

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	loadButtons();
	loadInterfaces = true;
	loadAircraftBlip2();
	loadUnknownBlip();

	//PlaySound(L"Sounds/atcmessage.wav",NULL,SND_FILENAME|SND_ASYNC);
	PlaySound(MAKEINTRESOURCE(IDW_SOUND1), NULL, SND_RESOURCE | SND_ASYNC);

	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
	wincl.cbSize = sizeof(WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon(hThisInstance, MAKEINTRESOURCE(MyIcon));//LoadIcon (NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon(hThisInstance, MAKEINTRESOURCE(MyIcon));
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;                 /* No menu */
	wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                      /* structure or the window instance */
	/* Use Windows's default color as the background of the window */
	wincl.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx(&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindowEx(
		0,                   /* Extended possibilites for variation */
		szClassName,         /* Classname */
		L"VAX",       /* Title Text */
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		WIDTH,                 /* The programs width */
		HEIGHT,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /* No menu */
		hThisInstance,       /* Program Instance handler */
		NULL                 /* No Window Creation data */
	);

	ShowWindow(hwnd, SW_SHOW);
	SetFocus(hwnd);

	/* Run the message loop. It will run until GetMessage() returns 0 */
	while (GetMessage(&messages, NULL, 0, 0))
	{
		/* Translate virtual-key messages into character messages */
		TranslateMessage(&messages);
		/* Send message to WindowProcedure */
		DispatchMessage(&messages);
	}

	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)                  /* handle the messages */
	{
	case WM_CREATE: {
		HMENU hMenuBar = CreateMenu();
		HMENU hFile = CreateMenu();
		HMENU hSettings = CreateMenu();
		HMENU hHelp = CreateMenu();

		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSettings, L"&Settings");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

		AppendMenu(hFile, MF_STRING, ID_FILE_CONNECT, L"&Connect to Sever...");
		AppendMenu(hFile, MF_STRING, ID_FILE_OPEN, L"&Open ADX File...");
		AppendMenu(hFile, MF_STRING, ID_FILE_EXIT, L"&Exit");
		AppendMenu(hHelp, MF_STRING, ID_HELP_ABOUT, L"&About...");

		SetMenu(hwnd, hMenuBar);

		//opengl stuff

		hWnd = hwnd;

		if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
			CAPS = true;
		else
			CAPS = false;

		CreateThread(NULL, 0, OpenGLThread, hwnd, 0, 0);
		CreateThread(NULL, 0, EventThread1, hwnd, 0, NULL);
		userStorage1.resize(MAX_AIRCRAFT_SIZE);

		/*User *user1 = new User("AAL2", PILOT_CLIENT, 0, 0);
		Aircraft *cur = new Aircraft();
		user1->setAircraft(cur);
		if (cur != NULL) {
		cur->lock();
		cur->setHeavy(true);
		cur->setCallsign("AAL2");
		cur->setLatitude(25.800704);
		cur->setLongitude(-80.300770);
		cur->setSpeed(0.0);
		cur->setHeading(87.0);
		cur->setRenderCallsign(true);
		AcfMap[cur->getCallsign()] = cur;
		cur->unlock();
		}
		userStorage1[0] = user1;
		user1 = new User("EGF4427", PILOT_CLIENT, 0, 0);
		cur = new Aircraft();
		user1->setAircraft(cur);
		if (cur != NULL) {
		cur->lock();
		cur->setHeavy(false);
		cur->setCallsign("EGF4427");
		cur->setLatitude(25.798267);
		cur->setLongitude(-80.282544);
		cur->setSpeed(0.0);
		cur->setHeading(180.0);
		cur->setRenderCallsign(true);
		AcfMap[cur->getCallsign()] = cur;
		cur->unlock();
		}
		userStorage1[2] = user1;*/
		break;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		RECT rect;
		if (!fixed_set) {
			FIXED_CLIENT_WIDTH = width;
			FIXED_CLIENT_HEIGHT = height;
			fixed_set = true;
		}
		if (GetWindowRect(hwnd, &rect))
		{
			int width2 = rect.right - rect.left;
			int height2 = rect.bottom - rect.top;
			WIDTH = width2;
			HEIGHT = height2;
		}
		CLIENT_WIDTH = width;
		CLIENT_HEIGHT = height;

		resize = true;
		renderButtons = true;
		renderLegend = true;
		renderInterfaces = true;
		renderDrawings = true;
		renderConf = true;
		renderAircraft = true;
	}
	break;
	case WM_MOUSEMOVE:
	{
		MOUSE_X = LOWORD(lParam);
		MOUSE_Y = HIWORD(lParam);
		MOUSE_MOVE = true;
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_CONNECT:
		{
			handleConnect();
		}
		break;
		case ID_FILE_OPEN:
		{
			OPENFILENAME ofn;
			TCHAR szFileName[MAX_PATH] = L"";

			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = L"ASDE-X Files (*.adx)\0*.adx\0All Files (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = L"adx";

			if (GetOpenFileName(&ofn))
			{
				std::wstring wide(szFileName);
				std::string final1(wide.begin(), wide.end());
				if (FileReader::LoadADX(final1)) {
					resize = true;
					renderSector = true;
					renderButtons = true;
					renderAllCallsigns = true;
					renderLegend = true;
					renderInterfaces = true;
					renderDrawings = true;
					renderConf = true;
					renderAircraft = true;
				}
			}
		}
		break;
		case ID_FILE_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case ID_HELP_ABOUT:
		{
			int ret = DialogBox(GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
			if (ret == IDOK) {
				//MessageBox(hwnd, L"Dialog exited with IDOK.", L"Notice",
				//MB_OK | MB_ICONINFORMATION);
			}
			else if (ret == -1) {
				//MessageBox(hwnd, L"Dialog failed!", L"Error",
				//MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		case ID_BUTTON:
		{
		}
		break;
		}
		break;
	case WM_DESTROY:
	{
		done = true;
		PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
	}
	break;
	case WM_LBUTTONDOWN:
	{
		WORD x = LOWORD(lParam), y = (CLIENT_HEIGHT - HIWORD(lParam));
		for (InterfaceFrame* frame : frames) {
			if (frame && frame->render) {
				for (ChatInterface* inter1 : frame->interfaces) {
				}
				ChildFrame* clicked1 = NULL;
				for (ChildFrame* children : frame->children) {
					if (children) {
						for (ChatInterface* inter2 : children->child_interfaces) {
							if (inter2->isBounds()) {
								int vertx[4] = { inter2->getStartX(), inter2->getStartX(), inter2->getEndX(), inter2->getEndX() };
								int verty[4] = { inter2->getStartY(), inter2->getEndY(), inter2->getEndY(), inter2->getStartY() };
								bool clicked = pnpoly(4, vertx, verty, x, y);
								if (clicked) {
									clicked1 = children;
								}
							}
						}
					}
				}
				if (clicked1 != NULL) {
					if (clicked1->type == INPUT_FIELD && !((InputField*)clicked1)->editable) {
						//dont do anything with non editable field
						printf("What");
					}
					else {
						if (focusChild != clicked1) {
							clicked1->setFocus();
						}
						if (focusChild == clicked1) {
							clicked1->doAction();
						}
					}
				}
			}
		}
	}
	break;
	case WM_KEYDOWN:
	{
		//std::cout << wParam << std::endl;
		if (wParam == VK_F1) {
		}
		else if (wParam == VK_F2) {
			char* cmd = ".AN ";
			pass_command(cmd);
		}
		else if (wParam == VK_F3) {
		}
		else if (wParam == VK_F4) {
		}
		else if (wParam == VK_F5) {
		}
		else if (wParam == VK_F6) {
			char* cmd = ".SS ";
			pass_command(cmd);
		}
		else if (wParam == VK_F7) {
		}
		else if (wParam == VK_F8) {
		}
		else if (wParam == VK_F9) {
		}
		else if (wParam == VK_F10) {
		}
		else if (wParam == VK_F11) {
		}
		else if (wParam == VK_F12) {
		}
		else if (wParam == VK_CAPITAL) {
			if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
				CAPS = true;
			else
				CAPS = false;
		}
		else if (wParam == VK_SHIFT) {
			if (!SHIFT_DOWN) {
				if (!CAPS) {
					CAPS = true;
				}
				else {
					CAPS = false;
				}
				SHIFT_DOWN = true;
			}
		}
		else if (wParam == VK_TAB) {
		}
		else if (wParam == VK_RETURN) {
			if (focusChild != NULL) {
				int type = focusChild->type;
				if (type == INPUT_FIELD) {
					InputField* focusField = (InputField*)focusChild;
					if (focusField == textField) {// main chat
						if (focusField->input.size() > 1) {
							focusField->popInput();
							if (processCommands(focusField->input)) {
								focusField->clearInput();
								focusField->pushInput(true, input_cursor);
								renderInputText = true;
							}
							else {
								for (size_t i = 0; i < userStorage1.size(); i++) {
									User* curUsr = userStorage1[i];
									if (curUsr != NULL && curUsr != USER) {
										sendUserMessage(*curUsr, focusField->input);
									}
								}
								main_chat->addLine(USER->getIdentity()->callsign + std::string(": ") + focusField->input, CHAT_TYPE::MAIN);
								renderDrawings = true;
								focusField->clearInput();
								focusField->pushInput(true, input_cursor);
								renderInputText = true;
							}
						}
					}
				}
			}
		}
		else if (wParam == VK_LEFT) {
			if (focusChild != NULL) {
				int type = focusChild->type;
				if (type == COMBO_BOX) {
					ComboBox* box = (ComboBox*)focusChild;
					if (box->pos > 0) {
						box->pos--;
						renderDrawings = true;
					}
				}
				else if (type == INPUT_FIELD) {
				}
			}
		}
		else if (wParam == VK_RIGHT) {
			if (focusChild != NULL) {
				int type = focusChild->type;
				if (type == COMBO_BOX) {
					ComboBox* box = (ComboBox*)focusChild;
					if (box->pos < (box->options.size() - 1)) {
						box->pos++;
						renderDrawings = true;
					}
				}
				else if (type == INPUT_FIELD) {
				}
			}
		}
		else if (wParam == VK_UP) {
		}
		else if (wParam == VK_DOWN) {
		}
		else if (wParam == VK_BACK) {
			if (focusChild != NULL && focusChild->type == INPUT_FIELD) {
				InputField* focusField = (InputField*)focusChild;
				if (focusField->editable) {
					if (focusField->input.size() > 1) {
						focusField->popInput();
						focusField->popInput();
						focusField->pushInput(true, input_cursor);
						renderInputText = true;
					}
				}
			}
		}
		else {
			char c = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
			char c2;
			if (!CAPS)
				c2 = tolower(c);
			else {
				char spec = NULL;
				switch (wParam) {
				case 49:
					spec = '!';
					break;
				case 191:
					spec = '?';
					break;
				case 57:
					spec = '(';
					break;
				case 48:
					spec = ')';
					break;
				case 189:
					spec = '_';
					break;
				case 186:
					spec = ':';
					break;
				case 222:
					spec = '"';
					break;
				case 50:
					spec = '@';
					break;
				case 51:
					spec = '#';
					break;
				case 52:
					spec = '$';
					break;
				case 53:
					spec = '%';
					break;
				case 56:
					spec = '*';
					break;
				default:
					break;
				}
				if (spec != NULL)
					c2 = spec;
				else
					c2 = c;
			}
			if (focusChild != NULL && focusChild->type == INPUT_FIELD) {
				InputField& focusField = *(InputField*)focusChild;
				if (focusField.editable) {
					if (focusField.input.size() > 0) {
						focusField.popInput();
					}
					focusField.pushInput(false, c2);
					focusField.pushInput(true, input_cursor);
					renderInputText = true;
				}
			}
		}
	}
	break;
	case WM_KEYUP:
	{
		if (wParam == VK_CAPITAL) {
			if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
				CAPS = true;
			else
				CAPS = false;
		}
		if (wParam == VK_SHIFT) {
			if (SHIFT_DOWN) {
				if (CAPS) {
					CAPS = false;
				}
				SHIFT_DOWN = false;
			}
		}
	}
	break;
	default:                      /* for messages that we don't deal with */
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

void handleConnect() {
	if (_openedframe && !_openedframe->multi_open) {
		return;
	}
	if (connected) {
		return;
	}
	if (connectFrame == NULL) {
		connectFrame = new InterfaceFrame(CONNECT_INTERFACE);
		connectFrame->title = "CONNECT";
		int width = 300, x = (CLIENT_WIDTH / 2) - (width / 2);
		int height = 200, y = (CLIENT_HEIGHT / 2) - (height / 2);
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
		connectFrame->doOpen(CONNECT_INTERFACE, false);
	}
	else {
		if (!connectFrame->render) {
			connectFrame->doOpen(CONNECT_INTERFACE, false);
		}
	}
}

bool processCommands(std::string command)
{
	if (boost::starts_with(command, ".SS")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			std::string call_sign = array3[1];
			std::unordered_map<std::string, User*>::const_iterator got = users_map.find(call_sign);
			if (got != users_map.end()) {
				User& user = *got->second;
				if (user.getIdentity()->type == PILOT_CLIENT) {
					FlightPlan& fp = *user.getAircraft()->getFlightPlan();
					sendFlightPlanRequest(user);
					if (fp.cycle) {
						Load_FlightPlan_Interface(user, true);
					}
					else {
						//TODO Change to No flight plan filed, rather than Unknown User
						Load_Known_No_FlightPlan_Interface(user, true);
					}
				}
			}
			else {
				Load_Unknown_FlightPlan_Interface((char*)call_sign.c_str(), true);
			}
		}
		return true;
	}
	else if (boost::starts_with(command, ".AN")) {
		return true;
	}
	else if (boost::starts_with(command, ".omir")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			std::string id = array3[1];
			Mirror* mir = mirrors_storage.at(id);
			bool opened = false;
			auto it = std::find(mirrors.begin(), mirrors.end(), mir);
			if (it != mirrors.end())
				opened = true;
			if (!opened)
			{
				mir->renderBorder = true;
				mir->renderSector = true;
				mir->renderAircraft = true;
				mirrors.push_back(mir);
			}
		}
		return true;
	}
	else if (boost::starts_with(command, ".cmir")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			std::string id = array3[1];
			Mirror* mir = mirrors_storage.at(id);
			mirrors.erase(std::remove(mirrors.begin(), mirrors.end(), mir), mirrors.end());
		}
		return true;
	}
	return false;
}

void connect() {
	if (intter->connectNew(hWnd, "127.0.0.1", 4403)) {
		connected = true;
		Stream stream = Stream(200);
		int type = USER->getIdentity()->type;
		intter->hand_shake = true;
		intter->current_op = 45;
		stream.createFrameVarSizeWord(45);
		stream.writeDWord(proto_version);
		char* callsign = s2ca1(USER->getIdentity()->callsign);
		char* fullname = s2ca1(USER->getIdentity()->login_name);
		char* username = s2ca1(USER->getIdentity()->username);
		char* pass = s2ca1(USER->getIdentity()->password);
		stream.writeString(callsign);
		stream.writeString(fullname);
		stream.writeString(username);
		stream.writeString(pass);
		stream.writeByte(USER->getIdentity()->controller_rating);
		stream.writeByte(USER->getIdentity()->pilot_rating);
		stream.writeQWord(1000);
		stream.writeQWord(doubleToRawBits(USER->getLatitude()));
		stream.writeQWord(doubleToRawBits(USER->getLongitude()));
		stream.writeWord(600);
		stream.writeByte(type);
		if (type == PILOT_CLIENT) {
			stream.writeString("King Air 350");
			stream.writeString("0000");
			stream.writeByte(0);
		}
		stream.endFrameVarSizeWord();
		intter->sendMessage(&stream);
		intter->startT(hWnd);
	}
}

void disconnect()
{
}

DWORD WINAPI EventThread1(LPVOID lpParameter) {
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	while (!done) {
		start = boost::posix_time::microsec_clock::local_time();
		event_manager1->update();
		end = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration time2 = end - start;
		long long time1 = 30L;
		long long time = time1 - time2.total_milliseconds();
		if (time < 1) {
			time = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time));
	}
	return 0;
}

DWORD WINAPI OpenGLThread(LPVOID lpParameter) {
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;

	hDC = BeginPaint(hWnd, &ps);
	// Setup pixel format for the device context
	SetPixelFormat(hDC);
	// Create a rendering context associated to the device context

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		//KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Unabled to Create Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		//KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	InitOpenGL();
	std::cout.precision(10);
	Event& position_updates = ConfigUpdates();
	position_updates.eAction.setTicks(0);
	event_manager1->addEvent(&position_updates);
	while (!done) {
		start = boost::posix_time::microsec_clock::local_time();
		ResizeGLScene();
		DrawGLScene();
		for (Mirror* mirror : mirrors) {
			if (mirror)
			{
				ResizeMirrorGLScene(*mirror);
				DrawMirrorScenes(*mirror);
			}
		}
		if (mirrors.size() > 0)
			ResizeGLScene();
		DrawInterfaces();
		resetFlags();
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err << std::endl;
		}
		SwapBuffers(hDC);
		end = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration time2 = end - start;
		long long time1 = (1000 / LOCKED_FPS);
		long long time = time1 - time2.total_milliseconds();
		if (time < 1) {
			time = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time));
	}
	return 0;
}

void pass_command(char* cmd) {
	InterfaceFrame* frame = frames[MAIN_CHAT_INTERFACE];
	if (frame && frame->index == MAIN_CHAT_INTERFACE && frame->render) {
		ChildFrame* child = frame->children[MAIN_CHAT_INPUT];
		if (child && child->type == INPUT_FIELD && child->focus) {
			InputField& input_box = *(InputField*)child;

			input_box.setInput(cmd);
			input_box.pushInput(true, input_cursor);
			renderInputText = true;
		}
	}
}

void pass_chars(char* chars) {
	InterfaceFrame* frame = frames[MAIN_CHAT_INTERFACE];
	if (frame && frame->index == MAIN_CHAT_INTERFACE && frame->render) {
		ChildFrame* child = frame->children[MAIN_CHAT_INPUT];
		if (child && child->type == INPUT_FIELD && child->focus) {
			InputField& input_box = *(InputField*)child;
			input_box.pass_characters(chars);
		}
	}
}

void resetFlags() {

}
