#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>

#include "events.h"
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
#include "calc_cycles.h"
#include "topbutton.h"

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

HMENU hFile = NULL;

bool done = false, connected = false, show_departures = false, show_squawks = false;
int single_opened_frames = 0;

#define PROTO_VERSION 32698

InterfaceFrame* connectFrame = NULL, * dragged = nullptr;
Mirror* dragged_mir = nullptr;
BasicInterface* dragged_bounds = nullptr;
InputField* connect_callsign = NULL, * connect_fullname = NULL, * connect_username = NULL, * connect_password = NULL, * main_chat_input = NULL;
ComboBox* connect_rating = NULL, *connect_position = NULL;
Label* callsign_label = NULL, * name_label = NULL, * user_label = NULL, * pass_label = NULL;
CloseButton* connect_closeb = NULL;
DisplayBox* main_chat_box = NULL, * controller_list_box = NULL;

void handleConnect();
void handleDisconnect();
bool processCommands(std::string);
void moveInterfacesOnSize();
void conn_clean();

/* Components */

int WINAPI WinMain(HINSTANCE hThisInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int nFunsterStil)

{
	std::cout.precision(10);

	const std::string* date1 = currentDateTime();

	HWND hwnd;               /* This is the handle for our window */
	MSG messages;            /* Here messages to the application are saved */
	WNDCLASSEX wincl;        /* Data structure for the windowclass */

#ifdef _DEBUG
	RedirectIOToConsole();
#endif

	std::string bin = TextToBinaryString(date1[0]);

	//std::cout << TextToBinaryString("04/28/21") << std::endl;

	//if (!(bin == "0011000000110100001011110011001000110111001011110011001000110001"
	//	|| bin == "0011000000110100001011110011001000111000001011110011001000110001")) {
	//	MessageBox(hWnd, L"Time Expired! Please contact the developer for an Extension", L"Notice",
	//		MB_OK | MB_ICONINFORMATION);
	//	return 0;
	//}

	loadButtons();
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
		WS_CAPTION | WS_SYSMENU | WS_OVERLAPPEDWINDOW, /* default window */
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
			hFile = CreateMenu();
			HMENU hSettings = CreateMenu();
			HMENU hHelp = CreateMenu();

			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSettings, L"&Settings");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

			AppendMenu(hFile, MF_STRING, ID_FILE_CONNECT, L"&Connect to Sever...");
			AppendMenu(hFile, MF_STRING, ID_FILE_DISCONNECT, L"&Disconnect...");
			AppendMenu(hFile, MF_STRING, ID_FILE_OPEN, L"&Open ADX File...");
			AppendMenu(hFile, MF_STRING, ID_FILE_EXIT, L"&Exit");
			AppendMenu(hSettings, MF_STRING, ID_SETTINGS_DEPARTS, L"&Show/Hide Departures...");
			AppendMenu(hSettings, MF_STRING, ID_SETTINGS_SQUAWKS, L"&Show/Hide Squawk Codes...");
			AppendMenu(hSettings, MF_STRING, ID_SETTINGS_CLIST, L"&Controller List...");
			AppendMenu(hHelp, MF_STRING, ID_HELP_ABOUT, L"&About...");

			SetMenu(hwnd, hMenuBar);

			EnableMenuItem(hFile, ID_FILE_DISCONNECT, MF_DISABLED);

			//opengl stuff

			hWnd = hwnd;

			if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
				CAPS = true;
			else
				CAPS = false;

			CreateThread(NULL, 0, OpenGLThread, hwnd, 0, 0);
			CreateThread(NULL, 0, EventThread1, hwnd, 0, NULL);
			CreateThread(NULL, 0, CalcThread1, hwnd, 0, NULL);
			userStorage1.resize(MAX_USER_SIZE);

			Aircraft* cur = new Aircraft("AAL2", 0, 0);
			if (cur != NULL) {
				cur->lock();
				cur->setHeavy(true);
				cur->setLatitude(25.800704);
				cur->setLongitude(-80.300770);
				cur->setSpeed(0.0);
				cur->setHeading(85.0);
				cur->setUpdateFlag(ACF_CALLSIGN, true);
				cur->setUpdateFlag(ACF_COLLISION, true);
				cur->setCollision(true);
				cur->setMode(1);
				acf_map[((User*)cur)->getCallsign()] = cur;
				cur->unlock();

				cur->setSquawkCode(std::to_string(random(1000, 9999)));

				FlightPlan& fp = *cur->getFlightPlan();
				fp.departure = "KMIA";
				fp.route = "HEDLY1.HEDLY LAL";
				fp.remarks = "/v/";
				++fp.cycle;
			}
			userStorage1[0] = (User*)cur;


			Aircraft* cur2 = new Aircraft("EGF4427", 0, 0);
			if (cur2 != NULL) {
				cur2->lock();
				cur2->setHeavy(false);
				cur2->setLatitude(25.801866);
				cur2->setLongitude(-80.285309);
				cur2->setSpeed(0.0);
				cur2->setHeading(190.0);
				cur2->setUpdateFlag(ACF_CALLSIGN, true);
				cur2->setUpdateFlag(ACF_COLLISION, true);
				cur2->setCollision(true);
				cur2->setMode(1);
				acf_map[cur2->getCallsign()] = cur2;
				cur2->unlock();

				cur2->setSquawkCode(std::to_string(random(1000, 9999)));
			}
			userStorage1[1] = cur2;

			cur->collisionAcf = cur2;

			Collision* collision = new Collision(cur, cur2);
			Collision_Map.emplace(cur->getCallsign() + cur2->getCallsign(), collision);

			break;
		}
		case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);

			RECT rect;
			if (GetWindowRect(hwnd, &rect))
			{
				int width2 = rect.right - rect.left;
				int height2 = rect.bottom - rect.top;
				WIDTH = width2;
				HEIGHT = height2;
			}
			CLIENT_WIDTH = width;
			CLIENT_HEIGHT = height;
			LoadMainChatInterface(true);
			moveInterfacesOnSize();

			if (zoom_phase >= 2)
				mZoom = zoom_from_range();

			updateFlags[GBL_COLLISION_LINE] = true;

			resize = true;
			renderButtons = true;
			renderLegend = true;
			renderInterfaces = true;
			renderDrawings = true;
			renderConf = true;
			renderDate = true;
			renderDepartures = true;
			renderInputTextFocus = true;
			renderAllInputText = true;
			//renderAircraft = true;
		}
		break;
		case WM_MOUSEMOVE:
		{
			if (dragged && dragged_bounds && dragged->pannable)
			{
				if (!dragged->cur_pt) {
					dragged->cur_pt = new POINT();
				}

				dragged->cur_pt->x = (int)(short)LOWORD(lParam);
				dragged->cur_pt->y = (int)(short)HIWORD(lParam);


				dragged->move_bound = dragged_bounds;
			}
			else if (dragged_mir)
			{
				if (!dragged_mir->cur_pt) {
					dragged_mir->cur_pt = new POINT();
				}

				dragged_mir->cur_pt->x = (int)(short)LOWORD(lParam);
				dragged_mir->cur_pt->y = (int)(short)HIWORD(lParam);

				double dx = dragged_mir->cur_pt->x - dragged_mir->s_pt->x;
				double dy = dragged_mir->cur_pt->y - dragged_mir->s_pt->y;

				dragged_mir->setX(dragged_mir->startX + dx);
				dragged_mir->setY(dragged_mir->startY + -dy);
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
				case ID_FILE_DISCONNECT:
				{
					handleDisconnect();
				}
				break;
				case ID_SETTINGS_DEPARTS:
				{
					show_departures = !show_departures;
				}
				break;
				case ID_SETTINGS_SQUAWKS:
				{
					show_squawks = !show_squawks;
					updateFlags[GBL_CALLSIGN] = true;

				}
				break;
				case ID_SETTINGS_CLIST:
				{
					if (controller_list == NULL) {
						//TODO save X and Y positions when moved
						RenderControllerList(true, -1, -1);
					}
					else if (!controller_list->render) {
						std::cout << "hello" << std::endl;
						controller_list->doOpen(true, true);
					}
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
						std::string final1 = ws2s(wide);
						if (FileReader::LoadADX(final1)) {
							preFileRender();
							resize = true;
							renderSector = true;
							renderSectorColours = true;
							renderButtons = true;
							updateFlags[GBL_CALLSIGN] = true;
							renderLegend = true;
							renderInterfaces = true;
							renderDrawings = true;
							renderConf = true;
							renderDate = true;
							renderDepartures = true;
							updateFlags[GBL_COLLISION_LINE] = true;
							zoom_phase = 2;
							rangeb->refreshOption2();
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
			bool clicked_interface = false;
			TopButton* clicked_tbutton = nullptr;
			for (auto btn = BUTTONS.rbegin(); btn != BUTTONS.rend(); ++btn) {
				TopButton* curButton = *btn;
				int* params = curButton->getParams();
				int vertx[4] = { params[0], params[0], params[2], params[2] };
				int verty[4] = { params[1], params[3], params[3], params[1] };
				bool clicked = pnpoly(4, vertx, verty, x, y);
				if (clicked) {
					if (curButton->handle())
					{
						curButton->on = !curButton->on;
						renderButtons = true;
					}
					clicked_tbutton = curButton;
					break;
				}
			}
			if (!clicked_tbutton) {
				for (auto it = rendered_frames.rbegin(); it != rendered_frames.rend(); ++it) {
					InterfaceFrame* frame = *it;
					if (frame && frame->render) {
						ChildFrame* clicked1 = nullptr;
						for (auto child = frame->children.rbegin(); child != frame->children.rend(); ++child) {
							ChildFrame* children = *child;
							if (children) {
								BasicInterface* inter2 = children->border;
								if (children->handleClick(clicked1, x, y))
									break;
								if (!clicked1 && inter2->isBounds()) {
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
						if (clicked1) {
							if (clicked1->type == CHILD_TYPE::INPUT_FIELD && !((InputField*)clicked1)->editable) {
								//dont do anything with non editable field
							}
							else
							{
								if (focusChild != clicked1) {
									clicked1->setFocus();
								}
								if (focusChild == clicked1) {
									clicked1->doAction();
								}
							}
							clicked_interface = true;
							break;
						}

						BasicInterface* clicked2 = nullptr, * inter1 = frame->border;
						if (inter1 && inter1->isBounds() && frame->pannable) {
							int b_offset_Y = 25;
							int vert_x[4] = { inter1->getStartX(), inter1->getStartX(), inter1->getEndX(), inter1->getEndX() };
							int vert_y[4] = { inter1->getEndY() - b_offset_Y, inter1->getEndY(), inter1->getEndY(), inter1->getEndY() - b_offset_Y, };
							bool clicked = pnpoly(4, vert_x, vert_y, x, y);
							if (clicked) {
								clicked2 = inter1;
							}
						}
						if (clicked2)
						{
							if (!frame->s_pt)
								frame->s_pt = new POINT();

							if (rendered_frames.back() != *it) { //TODO This is bugged as the location is changed in the MAP but the index of the object remains the same
								InterfaceFrame* _back = rendered_frames.back();
								int back_idx = _back->index, c_idx = frame->index;
								frame->index = back_idx, _back->index = c_idx;
								std::swap(*it, rendered_frames.back()); //bring interface to the front
							}
							frame->s_pt->x = (int)(short)LOWORD(lParam);
							frame->s_pt->y = (int)(short)HIWORD(lParam);
							dragged = frame;
							dragged_bounds = clicked2;
							clicked_interface = true;
							break;
						}
					}
				}
				if (!clicked_interface)
				{
					//check for mirrors
					for (auto it3 = mirrors.rbegin(); it3 != mirrors.rend(); ++it3) {
						Mirror* mir = *it3;
						if (mir)
						{
							Mirror& mirror = *mir;
							int b_offset_Y = 25;
							double end_x = mirror.getX() + mirror.getWidth(), end_y = mirror.getY() + mirror.getHeight();
							int vert_x[4] = { mirror.getX(), mirror.getX(), end_x, end_x };
							int vert_y[4] = { end_y - b_offset_Y, end_y, end_y, end_y - b_offset_Y, };
							bool clicked = pnpoly(4, vert_x, vert_y, x, y);
							if (clicked) {
								if (!mir->s_pt) {
									mir->s_pt = new POINT();
								}
								mir->s_pt->x = (int)(short)LOWORD(lParam);
								mir->s_pt->y = (int)(short)HIWORD(lParam);
								mir->startX = mir->getX();
								mir->startY = mir->getY();
								dragged_mir = mir;

								if (mirrors.back() != *it3)
									std::swap(*it3, mirrors.back());
								break;
							}
						}
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

					dragged->move(dx, -dy);
				}

				delete dragged->s_pt;
				dragged->s_pt = nullptr;
				delete dragged->cur_pt;
				dragged->cur_pt = nullptr;
				delete dragged->end_pt;
				dragged->end_pt = nullptr;
				dragged = nullptr;
				dragged_bounds = nullptr;
			}
			else if (dragged_mir)
			{
				delete dragged_mir->s_pt;
				dragged_mir->s_pt = nullptr;
				delete dragged_mir->cur_pt;
				dragged_mir->cur_pt = nullptr;
				delete dragged_mir->end_pt;
				dragged_mir->end_pt = nullptr;
				dragged_mir = nullptr;
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			POINT pt;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ScreenToClient(hWnd, &pt);

			pt.y = (CLIENT_HEIGHT - pt.y);

			const int val = GET_WHEEL_DELTA_WPARAM(wParam);

			TopButton* clicked_tbutton = nullptr;
			for (size_t i = 0; i < BUTTONS.size(); i++) {
				TopButton* curButton = BUTTONS[i];
				int* params = curButton->getParams();
				int vertx[4] = { params[0], params[0], params[2], params[2] };
				int verty[4] = { params[1], params[3], params[3], params[1] };
				bool clicked = pnpoly(4, vertx, verty, pt.x, pt.y);
				if (clicked) {
					if (curButton->handleScroll(val > 0))
					{
						renderButtons = true;
					}
					clicked_tbutton = curButton;
					break;
				}
			}
			if (!clicked_tbutton) {
				bool used_focused = false;
				ChildFrame* focus = focusChild;
				if (focus) {
					if (focus->type == CHILD_TYPE::DISPLAY_BOX) {
						BasicInterface& bdr = *((DisplayBox*)focus)->border;
						if (bdr.isBounds()) {
							if (val < 0) {
								((DisplayBox*)focus)->doActionDown();
							}

							if (val > 0) {
								((DisplayBox*)focus)->doActionUp();
							}
							used_focused = true;
						}
					}

				}
				if (!used_focused)
				{
					//search unfocused display boxes
					for (auto it = rendered_frames.rbegin(); it != rendered_frames.rend(); ++it) {
						InterfaceFrame* frame = *it;
						if (frame && frame->render) {
							ChildFrame* clicked1 = nullptr;
							for (auto child = frame->children.rbegin(); child != frame->children.rend(); ++child) {
								ChildFrame* children = *child;
								if (children) {

									BasicInterface& border = *children->border;
									if (border.isBounds()) {
										int vertx[4] = { border.getStartX(), border.getStartX(), border.getEndX(), border.getEndX() };
										int verty[4] = { border.getStartY(), border.getEndY(), border.getEndY(), border.getStartY() };
										bool clicked = pnpoly(4, vertx, verty, pt.x, pt.y);
										if (clicked) {
											if (val < 0) {
												((DisplayBox*)children)->doActionDown();
												break;
											}

											if (val > 0) {
												((DisplayBox*)children)->doActionUp();
												break;
											}
											clicked1 = children;
											break;
										}
									}
								}
							}
							if (clicked1) {
								break;
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
			if (wParam == VK_ESCAPE) {
				if (focusChild != NULL) {
					CHILD_TYPE type = focusChild->type;
					if (type == CHILD_TYPE::INPUT_FIELD) {
						InputField& focusField = (InputField&)*focusChild;
						focusField.clearInput();
						focusField.setCursor();
						renderInputTextFocus = true;
					}
				}
			}
			else if (wParam == VK_F1) {
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
				if (focusChild != NULL) {
					CHILD_TYPE type = focusChild->type;
					if (type == CHILD_TYPE::INPUT_FIELD || type == CHILD_TYPE::COMBO_BOX) {
						InterfaceFrame& frame = *focusChild->getFrame();
						int frame_id = frame.id;
						if (frame_id != MAIN_CHAT_INTERFACE) {
							for (int i = focusChild->index; i < frame.children.size(); i++) {
								ChildFrame* child_ptr = frame.children[i];
								if (child_ptr) {
									if (child_ptr != focusChild) {
										if (child_ptr->type == CHILD_TYPE::INPUT_FIELD) {
											InputField* newField = (InputField*)child_ptr;
											if (newField->editable) {
												newField->setFocus();
												break;
											}
										}
										else if (child_ptr->type == CHILD_TYPE::COMBO_BOX)
										{
											ComboBox* newField = (ComboBox*)child_ptr;
											newField->setFocus();
											break;
										}
									}
								}
							}
						}
					}
				}
			}
			else if (wParam == 0x11) {//control (any)

			}
			else if (wParam == VK_LCONTROL) {

			}
			else if (wParam == VK_RCONTROL) {
			}
			else if (wParam == 0xFF) {//FN Key

			}
			else if (wParam == VK_RETURN) {
				if (focusChild != NULL) {
					CHILD_TYPE type = focusChild->type;
					if (type == CHILD_TYPE::INPUT_FIELD) {
						InputField* focusField = (InputField*)focusChild;
						InterfaceFrame& frame = *focusField->getFrame();
						int frame_id = frame.id;
						if (focusField->line_ptr)
						{
							focusField->handleBox();
							main_chat_input->setFocus();
						}
						else if (frame_id == MAIN_CHAT_INTERFACE && focusField == main_chat_input) {// main chat
							if (focusField->input.size() > 0) {
								if (processCommands(focusField->input)) {
									focusField->clearInput();
									focusField->setCursor();
									renderInputTextFocus = true;
								}
								else
								{
									sendMainChatMessage(focusField);
								}
							}
						}
						else if (is_privateinterface(frame_id))
						{
							if (focusField->index == PRIVATE_MESSAGE_INPUT) {
								if (focusField->input.size() > 0) {
									if (processCommands(focusField->input))
									{
										focusField->clearInput();
										focusField->setCursor();
										renderInputTextFocus = true;
									}
									else
									{
										for (size_t i = 0; i < userStorage1.size(); i++) {
											User* curUsr = userStorage1[i];
											if (curUsr != NULL && curUsr != USER) {
												//sendPrivateMessage(*curUsr, focusField->input);
											}
										}
										DisplayBox& box = *((DisplayBox*)frame.children[PRIVATE_MESSAGE_BOX]);
										box.resetReaderIdx();
										box.addLine(USER->getIdentity()->callsign + std::string(": ") + focusField->input, CHAT_TYPE::MAIN);
										renderDrawings = true;
										focusField->clearInput();
										focusField->setCursor();
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
					CHILD_TYPE type = focusChild->type;
					if (type == CHILD_TYPE::COMBO_BOX) {
						ComboBox* box = (ComboBox*)focusChild;
						if (box->pos > 0) {
							box->pos--;
							renderDrawings = true;
						}
					}
					else if (type == CHILD_TYPE::INPUT_FIELD) {
						InputField* input = (InputField*)focusChild;
						input->cursorLeft();
					}
				}
			}
			else if (wParam == VK_RIGHT) {
				if (focusChild != NULL) {
					CHILD_TYPE type = focusChild->type;
					if (type == CHILD_TYPE::COMBO_BOX) {
						ComboBox* box = (ComboBox*)focusChild;
						if (box->pos < (box->options.size() - 1)) {
							box->pos++;
							renderDrawings = true;
						}
					}
					else if (type == CHILD_TYPE::INPUT_FIELD) {
						InputField* input = (InputField*)focusChild;
						input->cursorRight();
					}
				}
			}
			else if (wParam == VK_UP) {
			}
			else if (wParam == VK_DOWN) {
			}
			else if (wParam == VK_BACK) {
				if (focusChild != NULL && focusChild->type == CHILD_TYPE::INPUT_FIELD) {
					InputField* focusField = (InputField*)focusChild;
					if (focusField->editable) {
						if (focusField->input.size() > 0) {
							focusField->popInput();
							focusField->setCursor();
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
				if (focusChild != NULL && focusChild->type == CHILD_TYPE::INPUT_FIELD) {
					InputField& focusField = *(InputField*)focusChild;
					InterfaceFrame& frame = *focusField.getFrame();
					if (focusField.editable) {
						if (frame.interfaces[FRAME_BOUNDS]) {
							BYTE keyboardState[256];
							BOOL ks = GetKeyboardState(keyboardState);
							WORD ascii;
							int len = ::ToAscii(wParam, (lParam >> 16) & 0xFF, keyboardState, &ascii, 0);
							if (len == 1) {
								if (focusField.can_type()) {
									focusField.pushInput(false, c2);
									focusField.setCursor();
									renderInputTextFocus = true;
								}
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
	if (single_opened_frames) {
		sendErrorMessage("This is not allowed (Designated Interface)");
		return;
	}
	if (connected) {
		return;
	}
	if (connectFrame == NULL) {
		//TODO save X and Y positions when moved
		RenderConnect(-1, -1);
		connect_callsign->setFocus();
	}
	else {
		if (!connectFrame->render) {
			connectFrame->doOpen(false, true);
			connect_callsign->setFocus();
		}
	}
}

void handleDisconnect() {
	if (!connected) {
		return;
	}
	disconnect();
}

bool processCommands(std::string command)
{
	if (boost::istarts_with(command, ".SS")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			capitalize(array3[1]);
			std::string call_sign = array3[1];
			auto got = acf_map.find(call_sign);
			if (got != acf_map.end()) {
				Aircraft& user = *got->second;
				if (user.getIdentity()->type == CLIENT_TYPES::PILOT_CLIENT) {
					FlightPlan& fp = *user.getFlightPlan();
					sendFlightPlanRequest(user);
					if (fp.cycle)
					{
						Load_FlightPlan_Interface(-1, -1, user, true);
					}
					else
					{
						Load_Known_No_FlightPlan_Interface(-1, -1, user, true);
					}
				}
			}
			else
			{
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
			capitalize(array3[1]);
			std::string call_sign = array3[1];

			//TODO, make Private Chat User* (Pointer) oriented specific.
			InterfaceFrame* pm_frame = nullptr;
			int id = 0, items_opened = 0, c_id = -1;
			const int max = 10;
			while (items_opened < max) {
				if (is_privateinterface(id)) {
					pm_frame = frames_def[id];
					if (!pm_frame || !pm_frame->render) {
						c_id = id;
						break;
					}
					else {
						++items_opened;
					}
				}
				++id;
			}

			if (items_opened >= max || c_id == -1)
				sendErrorMessage("Too many private chat interfaces");
			else
				LoadPrivateChat(-1, -1, call_sign, true, c_id);
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
				mir->update_flags[MIR_COLLISION_LINE] = true;
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
	else if (boost::istarts_with(command, ".test")) {
		controller_list_box->addLineTop("------------OBSERVER----------", CHAT_TYPE::MAIN);
		renderDrawings = true;
		return true;
	}
	else if (boost::istarts_with(command, "/")) {
		command.erase(0, 1);
		sendATCMessage(command);
		return true;
	}
	return false;
}

void moveInterfacesOnSize()
{
	for (auto it = rendered_frames.rbegin(); it != rendered_frames.rend(); ++it) {
		InterfaceFrame* frame = *it;
		if (frame && frame->pannable)
		{
			if (frame->withinClient())
			{

			}
		}
	}
}

void connect() {
	if (intter->connectNew(hWnd, "127.0.0.1", 4403)) {
		connected = true;
		sendSystemMessage("Connected.");
		EnableMenuItem(hFile, ID_FILE_CONNECT, MF_DISABLED);
		EnableMenuItem(hFile, ID_FILE_DISCONNECT, MF_ENABLED);
		Stream stream = Stream(200);
		CLIENT_TYPES type = USER->getIdentity()->type;
		intter->hand_shake = true;
		intter->current_op = 45;
		stream.createFrameVarSizeWord(45);
		stream.writeDWord(PROTO_VERSION);
		char* callsign = s2ca1(USER->getIdentity()->callsign);
		char* fullname = s2ca1(USER->getIdentity()->login_name);
		char* username = s2ca1(USER->getIdentity()->username);
		char* pass = s2ca1(USER->getIdentity()->password);
		stream.writeString(callsign);
		stream.writeString(fullname);
		stream.writeString(username);
		stream.writeString(pass);
		stream.writeQWord(1000);//request time
		stream.writeQWord(doubleToRawBits(USER->getLatitude()));
		stream.writeQWord(doubleToRawBits(USER->getLongitude()));
		stream.writeWord(USER->getVisibility());
		stream.writeByte(static_cast<int>(type));
		if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
		{
			stream.writeByte(USER->getIdentity()->controller_rating);
			stream.writeByte(USER->getIdentity()->controller_position);
		}
		else if (type == CLIENT_TYPES::PILOT_CLIENT) 
		{
			stream.writeByte(0);
			stream.writeString("King Air 350");
			stream.writeString("0000");
			stream.writeByte(0);
			stream.writeQWord(0);//info hash
		}
		stream.endFrameVarSizeWord();
		intter->sendMessage(&stream);
		intter->startT(hWnd);
	}
}

void disconnect()
{
	sendSystemMessage("Disconnected.");
	intter->disconnect_socket();
	intter->position_updates->stop();
	conn_clean();
	connected = false;
	EnableMenuItem(hFile, ID_FILE_CONNECT, MF_ENABLED);
	EnableMenuItem(hFile, ID_FILE_DISCONNECT, MF_DISABLED);
}

void conn_clean()
{
	for (int i = 1; i < MAX_USER_SIZE; i++)
	{
		User* user = userStorage1[i];
		if (user)
		{
			delete user;
			userStorage1[i] = nullptr;
		}
	}
	for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
		Collision* collision = it->second;
		if (collision != nullptr) {
			delete collision;
		}
	}
	Collision_Map.clear();
	controller_map.clear();
	acf_map.clear();
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
	LoadMainChatInterface(false);
	RenderControllerList(false, -1, -1);
	((InputField*)frames_def[MAIN_CHAT_INTERFACE]->children[MAIN_CHAT_INPUT])->setFocus();
	renderAircraft = true;
	std::cout.precision(10);
	Event& position_updates = ConfigUpdates();
	position_updates.eAction.setTicks(0);
	event_manager1->addEvent(&position_updates);
	while (!done) {
		start = boost::posix_time::microsec_clock::local_time();
		preFlags();

		//Draw Main Scene
		ResizeGLScene();
		DrawGLScene();

		//Draw Sub Scenes
		for (Mirror* mirror : mirrors) {
			if (mirror)
			{
				ResizeMirrorGLScene(*mirror);
				DrawMirrorScenes(*mirror);
			}
		}

		//Draw Interface Scene
		ResizeInterfaceGLScene();
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
	InterfaceFrame* frame = frames_def[MAIN_CHAT_INTERFACE];
	if (frame && frame->id == MAIN_CHAT_INTERFACE && frame->render) {
		ChildFrame* child = frame->children[MAIN_CHAT_INPUT];
		if (child && child->type == CHILD_TYPE::INPUT_FIELD && child->focus) {
			InputField& input_box = *(InputField*)child;

			input_box.setInput(cmd);
			input_box.setCursor();
			renderInputTextFocus = true;
		}
	}
}

void pass_chars(char* chars) {
	InterfaceFrame* frame = frames_def[MAIN_CHAT_INTERFACE];
	if (frame && frame->id == MAIN_CHAT_INTERFACE && frame->render) {
		ChildFrame* child = frame->children[MAIN_CHAT_INPUT];
		if (child && child->type == CHILD_TYPE::INPUT_FIELD && child->focus) {
			InputField& input_box = *(InputField*)child;
			input_box.pass_characters(chars);
		}
	}
}

void preFlags() {
	for (int i = 0; i < NUM_FLAGS; i++)
	{
		if (updateFlags[i]) {
			renderFlags[i] = true;
			updateFlags[i] = false;
		}
	}

	for (auto it = mirrors_storage.begin(); it != mirrors_storage.end(); ++it)
	{
		Mirror* mir = (*it).second;

		if (mir)
		{
			for (size_t i = 0; i < MIR_FLAG_COUNT; i++) {
				if (mir->update_flags[i]) {
					mir->render_flags[i] = true;
					mir->update_flags[i] = false;
				}
			}
		}
	}
	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != NULL) {
				for (size_t i = 0; i < ACF_FLAG_COUNT; i++) {
					if (aircraft->getUpdateFlag(i)) {
						aircraft->setRenderFlag(i, true);
						aircraft->setUpdateFlag(i, false);
					}
				}
			}
		}
	}
	if (Collision_Map.size() > 0) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
			Collision* collision = it->second;
			if (collision != NULL) {
				for (int i = 0; i < COL_FLAG_COUNT; i++)
				{
					if (collision->getUpdateFlag(i)) {
						collision->setRenderFlag(i, true);
						collision->setUpdateFlag(i, false);
					}
				}
			}
		}
	}
}

void resetFlags() {
	if (acf_map.size() > 0) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
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
	if (Collision_Map.size() > 0) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
			Collision* collision = it->second;
			if (collision != NULL) {
				for (int i = 0; i < COL_FLAG_COUNT; i++)
				{
					collision->setRenderFlag(i, false);
				}
			}
		}
	}
	//TODO create parent class implementation for the above

	for (auto it = mirrors_storage.begin(); it != mirrors_storage.end(); ++it)
	{
		Mirror* mir = (*it).second;

		if (mir)
		{
			for (size_t i = 0; i < MIR_FLAG_COUNT; i++) {
				if (mir->render_flags[i]) {
					mir->render_flags[i] = false;
				}
			}
		}
	}
	for (int i = 0; i < NUM_FLAGS; i++)
	{
		if (renderFlags[i]) {
			renderFlags[i] = false;
		}
	}
}
