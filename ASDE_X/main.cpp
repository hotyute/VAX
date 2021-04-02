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
#include "interfaces.h"

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

InterfaceFrame* connectFrame = NULL, * dragged = nullptr;
BasicInterface* dragged_bounds = nullptr;
InputField* connect_callsign = NULL, * connect_fullname = NULL, * connect_username = NULL, * connect_password = NULL, * textField = NULL;
Label* callsign_label = NULL, * name_label = NULL, * user_label = NULL, * pass_label = NULL;
CloseButton* connect_closeb = NULL;
DisplayBox* main_chat = NULL;

void handleConnect();
bool processCommands(std::string);
bool click_arrow_bottom(BasicInterface& inter2, int x, int y, int arrow_bounds, int arrow_offset);
bool click_arrow_top(BasicInterface& inter2, int x, int y, int arrow_bounds, int arrow_offset);

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
	//PlaySound(MAKEINTRESOURCE(IDW_SOUND1), NULL, SND_RESOURCE | SND_ASYNC);

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

		User* user1 = new User("AAL2", PILOT_CLIENT, 0, 0);
		Aircraft* cur = new Aircraft();
		user1->setAircraft(cur);
		if (cur != NULL) {
			cur->lock();
			cur->setHeavy(true);
			cur->setCallsign("AAL2");
			cur->setLatitude(25.800704);
			cur->setLongitude(-80.300770);
			cur->setSpeed(0.0);
			cur->setHeading(85.0);
			cur->setUpdateFlag(CALLSIGN, true);
			cur->setUpdateFlag(COLLISION, true);
			cur->setCollision(true);
			cur->setMode(1);
			AcfMap[cur->getCallsign()] = cur;
			cur->unlock();
		}
		userStorage1[0] = user1;
		User* user2 = new User("EGF4427", PILOT_CLIENT, 0, 0);
		Aircraft* cur2 = new Aircraft();
		user2->setAircraft(cur2);
		if (cur2 != NULL) {
			cur2->lock();
			cur2->setHeavy(false);
			cur2->setCallsign("EGF4427");
			cur2->setLatitude(25.798267);
			cur2->setLongitude(-80.282544);
			cur2->setSpeed(0.0);
			cur2->setHeading(182.0);
			cur2->setUpdateFlag(CALLSIGN, true);
			cur2->setUpdateFlag(COLLISION, true);
			cur2->setCollision(true);
			cur2->setMode(1);
			AcfMap[cur2->getCallsign()] = cur2;
			cur2->unlock();
		}
		userStorage1[1] = user2;
		cur->collisionAcf = cur2;
		cur->setUpdateFlag(COLLISION_LINE, true);

		User* user3 = new User("DAL220", PILOT_CLIENT, 0, 0);
		Aircraft* cur3 = new Aircraft();
		user3->setAircraft(cur3);
		if (cur3 != NULL) {
			cur3->lock();
			cur3->setHeavy(false);
			cur3->setCallsign("DAL220");
			cur3->setLatitude(25.798429);
			cur3->setLongitude(-80.278852);
			cur3->setSpeed(0.0);
			cur3->setHeading(120.0);
			cur3->setUpdateFlag(CALLSIGN, true);
			cur3->setUpdateFlag(COLLISION, true);
			cur3->setMode(1);
			AcfMap[cur3->getCallsign()] = cur3;
			cur3->unlock();
		}
		userStorage1[2] = user3;
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
		//renderAircraft = true;
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (dragged && dragged_bounds && dragged->pannable)
		{
			RECT mainWindowRect;
			if (dragged->cur_pt)
				delete dragged->cur_pt;
			dragged->cur_pt = new POINT();
			int windowWidth, windowHeight;

			dragged->cur_pt->x = (int)(short)LOWORD(lParam);
			dragged->cur_pt->y = (int)(short)HIWORD(lParam);

			windowHeight = dragged_bounds->getStartY() - dragged_bounds->getEndY();
			windowWidth = dragged_bounds->getStartX() - dragged_bounds->getEndX();


			dragged->move_bound = dragged_bounds;
		}
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
					preFileRender();
					resize = true;
					renderSector = true;
					renderButtons = true;
					updateAllCallsigns = true;
					renderLegend = true;
					renderInterfaces = true;
					renderDrawings = true;
					renderConf = true;
					updateAllCollisionLines = true;
					zoom_phase = 2;
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
		for (auto it = frames.begin(); it != frames.end(); ++it) {
			InterfaceFrame* frame = *(it);
			if (frame && frame->render) {
				BasicInterface* clicked2 = NULL;
				for (BasicInterface* inter1 : frame->interfaces) {
					if (inter1 && inter1->isBounds() && frame->pannable) {
						int b_offset_Y = 25;
						int vert_x[4] = { inter1->getStartX(), inter1->getStartX(), inter1->getEndX(), inter1->getEndX() };
						int vert_y[4] = { inter1->getEndY() - b_offset_Y, inter1->getEndY(), inter1->getEndY(), inter1->getEndY() - b_offset_Y, };
						bool clicked = pnpoly(4, vert_x, vert_y, x, y);
						if (clicked) {
							clicked2 = inter1;
							break;
						}
					}
				}
				ChildFrame* clicked1 = NULL;
				for (ChildFrame* children : frame->children) {
					if (children) {
						for (BasicInterface* inter2 : children->child_interfaces) {
							if (children->type == DISPLAY_BOX) {
								int arrow_bounds = 15, arrow_offset = 3;
								if (click_arrow_bottom(*inter2, x, y, arrow_bounds, arrow_offset)) {
									((DisplayBox*)children)->doActionDown();
									break;
								}

								if (click_arrow_top(*inter2, x, y, arrow_bounds, arrow_offset)) {
									((DisplayBox*)children)->doActionUp();
									break;
								}

							}
							if (inter2->isBounds()) {
								int vertx[4] = { inter2->getStartX(), inter2->getStartX(), inter2->getEndX(), inter2->getEndX() };
								int verty[4] = { inter2->getStartY(), inter2->getEndY(), inter2->getEndY(), inter2->getStartY() };
								bool clicked = pnpoly(4, vertx, verty, x, y);
								if (clicked) {
									clicked1 = children;
									break;
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
				else if (clicked2 != NULL)
				{
					if (frame->s_pt)
						delete frame->s_pt;
					if (frames.back() != *it)
						std::swap(frames[it - frames.begin()], frames.back()); //bring interface to the front
					frame->s_pt = new POINT();
					frame->s_pt->x = (int)(short)LOWORD(lParam);
					frame->s_pt->y = (int)(short)HIWORD(lParam);
					dragged = frame;
					dragged_bounds = clicked2;
				}
			}
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (dragged && dragged_bounds)
		{
			if (dragged->cur_pt && dragged->s_pt)
			{

				int dx = (dragged->cur_pt->x - dragged->s_pt->x);
				int dy = (dragged->cur_pt->y - dragged->s_pt->y);

				if (dx != 0 || dy != 0)
				{
					if (dragged->pannable) {
						for (BasicInterface* inter1 : dragged->interfaces) {
							if (inter1)
							{
								inter1->setPosX(inter1->getPosX() + dx);
								inter1->setPosY(inter1->getPosY() + -dy);
								inter1->updateCoordinates();
							}
						}
						for (ChildFrame* children : dragged->children) {
							if (children) {
								for (BasicInterface* inter2 : children->child_interfaces) {
									inter2->setPosX(inter2->getPosX() + dx);
									inter2->setPosY(inter2->getPosY() + -dy);
									inter2->updateCoordinates();
								}
							}
						}

						dragged->renderAllInputText = true;
						dragged->renderAllLabels = true;
						renderDrawings = true;
						renderFocus = true;
						renderInterfaces = true;
					}
				}
			}

			dragged->s_pt = nullptr;
			dragged->cur_pt = nullptr;
			dragged->end_pt = nullptr;
			dragged = nullptr;
			dragged_bounds = nullptr;
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
					InterfaceFrame& frame = *focusField->getFrame();
					int frame_index = frame.index;
					if (frame_index == MAIN_CHAT_INTERFACE && focusField == textField) {// main chat
						if (focusField->input.size() > 1) {
							focusField->popInput();
							if (processCommands(focusField->input)) {
								focusField->clearInput();
								focusField->pushInput(true, input_cursor);
								renderInputTextFocus = true;
							}
							else
							{
								for (size_t i = 0; i < userStorage1.size(); i++) {
									User* curUsr = userStorage1[i];
									if (curUsr != NULL && curUsr != USER) {
										sendUserMessage(*curUsr, focusField->input);
									}
								}
								main_chat->resetReaderIdx();
								main_chat->addLine(USER->getIdentity()->callsign + std::string(": ") + focusField->input, CHAT_TYPE::MAIN);
								renderDrawings = true;
								focusField->clearInput();
								focusField->pushInput(true, input_cursor);
								renderInputTextFocus = true;
							}
						}
					}
					else if (frame_index == PRIVATE_MESSAGE_INTERFACE) {
						if (focusField->index == PRIVATE_MESSAGE_INPUT) {
							if (focusField->input.size() > 1) {
								focusField->popInput();
								if (processCommands(focusField->input))
								{
									focusField->clearInput();
									focusField->pushInput(true, input_cursor);
									renderInputTextFocus = true;
								}
								else
								{
									for (size_t i = 0; i < userStorage1.size(); i++) {
										User* curUsr = userStorage1[i];
										if (curUsr != NULL && curUsr != USER) {
											sendUserMessage(*curUsr, focusField->input);
										}
									}
									DisplayBox& box = *((DisplayBox*)frame.children[PRIVATE_MESSAGE_BOX]);
									box.resetReaderIdx();
									box.addLine(USER->getIdentity()->callsign + std::string(": ") + focusField->input, CHAT_TYPE::MAIN);
									renderDrawings = true;
									focusField->clearInput();
									focusField->pushInput(true, input_cursor);
									renderInputTextFocus = true;
								}
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
						renderInputTextFocus = true;
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
				InterfaceFrame& frame = *focusField.getFrame();
				if (focusField.editable) {
					if (frame.interfaces[FRAME_BOUNDS]) {
						if (focusField.can_type()) {
							if (focusField.input.size() > 0) {
								focusField.popInput();
							}
							focusField.pushInput(false, c2);
							focusField.pushInput(true, input_cursor);
							renderInputTextFocus = true;
						}
					}
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
		//TODO save X and Y positions when moved
		RenderConnect(-1, -1);
	} else {
		if (!connectFrame->render) {
			connectFrame->doOpen(false, true);
		}
	}
}

bool processCommands(std::string command)
{
	if (boost::istarts_with(command, ".SS")) {
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
						Load_FlightPlan_Interface(-1, -1, user, true);
					}
					else {
						//TODO Change to No flight plan filed, rather than Unknown User
						Load_Known_No_FlightPlan_Interface(-1, -1, user, true);
					}
				}
			}
			else {
				Load_Unknown_FlightPlan_Interface(-1, -1, (char*)call_sign.c_str(), true);
			}
		}
		return true;
	}
	else if (boost::istarts_with(command, ".AN")) {
		return true;
	}
	else if (boost::istarts_with(command, ".CHAT")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			std::string call_sign = array3[1];

			//TODO, make Private Chat User* (Pointer) oriented specific.
			InterfaceFrame* pm_frame = frames[PRIVATE_MESSAGE_INTERFACE];
			int index = PRIVATE_MESSAGE_INTERFACE;
			int max = 10;
			while (max > 0 && pm_frame && pm_frame->render) {
				pm_frame = frames[index];
				++index;
				--max;
			}
			LoadPrivateChat(-1, -1, call_sign, true, index);
		}
		return true;
	}
	else if (boost::istarts_with(command, ".omir")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			std::string id = array3[1];
			auto it = get_mir(mirrors_storage, id);
			bool found_store = it != mirrors_storage.end();
			if (!found_store)
				return true;

			Mirror* mir = (*it).second;
			bool opened = false;
			auto it2 = std::find(mirrors.begin(), mirrors.end(), mir);
			if (it2 != mirrors.end())
				opened = true;
			if (!opened)
			{
				mir->renderBorder = true;
				mir->renderAllCollisionLines = true;
				mirrors.push_back(mir);
			}
		}
		return true;
	}
	else if (boost::istarts_with(command, ".cmir")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			std::string id = array3[1];
			auto it = get_mir(mirrors_storage, id);
			bool found_store = it != mirrors_storage.end();
			if (!found_store)
				return true;
			Mirror* mir = (*it).second;
			auto it2 = std::find(mirrors.begin(), mirrors.end(), mir);
			if (it2 == mirrors.end())
				return true;
			mirrors.erase(it2);
		}
		return true;
	}
	else if (boost::istarts_with(command, "/")) {
		command.erase(0, 1);
		for (size_t i = 0; i < userStorage1.size(); i++) {
			User* curUsr = userStorage1[i];
			if (curUsr != NULL && curUsr != USER) {
				//sendUserMessage(*curUsr, command);
			}
		}
		main_chat->resetReaderIdx();
		ChatLine* c = new ChatLine(USER->getIdentity()->callsign + std::string(": ") + command, CHAT_TYPE::ATC);
		main_chat->addLine(c);
		c->playChatSound();
		renderDrawings = true;
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
	renderAircraft = true;
	std::cout.precision(10);
	Event& position_updates = ConfigUpdates();
	position_updates.eAction.setTicks(0);
	event_manager1->addEvent(&position_updates);
	while (!done) {
		start = boost::posix_time::microsec_clock::local_time();
		preFlags();
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
		{
			ResizeGLScene();
		}
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
			renderInputTextFocus = true;
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

void preFlags() {
	bool renderAllCollisionLines = false, renderAllCallsigns = false, renderAllCollision = false, renderAllCollTags = false;
	if (updateAllCollisionLines) {
		renderAllCollisionLines = true;
		updateAllCollisionLines = false;
	}
	if (updateAllCallsigns) {
		renderAllCallsigns = true;
		updateAllCallsigns = false;
	}
	if (updateAllCollision) {
		renderAllCollision = true;
		updateAllCollision = false;
	}
	if (updateAllCollTags) {
		renderAllCollTags = true;
		updateAllCollTags = false;
	}
	if (AcfMap.size() > 0) {
		std::map<std::string, Aircraft*>::iterator iter;
		for (iter = AcfMap.begin(); iter != AcfMap.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				if (aircraft->getUpdateFlag(COLLISION_LINE) || renderAllCollisionLines) {
					aircraft->setRenderFlag(COLLISION_LINE, true);
					aircraft->setUpdateFlag(COLLISION_LINE, false);
				}
				if (aircraft->getUpdateFlag(CALLSIGN) || renderAllCallsigns) {
					aircraft->setRenderFlag(CALLSIGN, true);
					aircraft->setUpdateFlag(CALLSIGN, false);
				}
				if (aircraft->getUpdateFlag(COLLISION) || renderAllCollision) {
					aircraft->setRenderFlag(COLLISION, true);
					aircraft->setUpdateFlag(COLLISION, false);
				}
				if (aircraft->getUpdateFlag(COLLISION_TAG) || renderAllCollTags) {
					aircraft->setRenderFlag(COLLISION_TAG, true);
					aircraft->setUpdateFlag(COLLISION_TAG, false);
				}
			}
		}
	}
	renderAllCollisionLines = false;
	renderAllCallsigns = false;
	renderAllCollision = false;
	renderAllCollTags = false;
}

void resetFlags() {
	if (AcfMap.size() > 0) {
		std::map<std::string, Aircraft*>::iterator iter;
		for (iter = AcfMap.begin(); iter != AcfMap.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				for (int i = 0; i < ACF_FLAG_COUNT; i++)
				{
					aircraft->setRenderFlag(i, false);
				}
			}
		}
	}
}

bool click_arrow_bottom(BasicInterface& inter2, int x, int y, int arrow_bounds, int arrow_offset) {
	bool clicked = false;
	int vertxt[4] = {
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset
	};
	int vertyt[4] = {
		inter2.getStartY(),
		inter2.getStartY() + arrow_bounds,
		inter2.getStartY() + arrow_bounds,
		inter2.getStartY()
	};
	bool clicked_bottom = pnpoly(4, vertxt, vertyt, x, y);
	if (clicked_bottom) {
		clicked = true;
	}
	return clicked;
}

bool click_arrow_top(BasicInterface& inter2, int x, int y, int arrow_bounds, int arrow_offset) {
	bool clicked = false;
	int vertx[4] = {
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset,
		inter2.getStartX() + inter2.getActualWidth() + (arrow_bounds)+arrow_offset
	};
	int verty[4] = {
		inter2.getStartY() + inter2.getActualHeight(),
		inter2.getStartY() + inter2.getActualHeight() - arrow_bounds,
		inter2.getStartY() + inter2.getActualHeight() - arrow_bounds,
		inter2.getStartY() + inter2.getActualHeight()
	};
	bool clicked_top = pnpoly(4, vertx, verty, x, y);
	if (clicked_top) {
		clicked = true;
	}
	return clicked;
}
