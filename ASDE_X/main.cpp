#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>

#include "events.h"
#include "projection.h"
#include "main.h"

#include <windowsx.h>

#include <gl/GLU.h>

#include "global_data.h"
#include "later.h"
#include "renderer.h"
#include "guicon.h"
#include "topbutton.h"
#include "dxfdrawing.h"
#include "tools.h"
#include "clinc2.h"
#include "usermanager.h"
#include "packets.h"
#include "interfaces.h"
#include "calc_cycles.h"
#include "flightplan.h"
#include "save.h"
#include "app_events.h"
#include "thread_pool.h"
#include "tempdata.h"
#include "gui/widgets/font_manager.h" // Add this

#include "gui/widgets/ui_manager.h"
#include "gui/widgets/opengl_ui_renderer.h" // Your concrete renderer
#include "gui/ui_windows/connect_window.h" // The first window you'll test
#include "gui/ui_windows/settings_window.h"
#include "gui/ui_windows/comms_window_widget.h"
#include "gui/ui_windows/controller_list_window.h"
#include "gui/widgets/font_keys.h"
#include "gui/ui_windows/private_chat_window.h"
// Potentially others as you convert them:
// #include "ui_windows/main_chat_window.h"
// ...

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
HMENU hSettings = NULL;

bool done = false, connected = false, show_departures = false, show_squawks = true;
int single_opened_frames = 0;

#define PROTO_VERSION 32698
constexpr int NUM_THREADS = 8;
Point2* MOUSE_POS = new Point2();
Point2* DISPLAY_MOUSE_POS = new Point2();
std::vector<MSG*> message_queue(500, NULL);
InterfaceFrame* connectFrame = NULL, * dragged = nullptr, * fp_frame = nullptr;
Mirror* dragged_mir = nullptr, * dragged_pos = nullptr;
BasicInterface* dragged_bounds = nullptr;
InputField* connect_callsign = NULL, * connect_fullname = NULL, * connect_username = NULL,
* connect_password = nullptr, * main_chat_input = nullptr, * terminal_input = nullptr, * squawk_input = nullptr;
ComboBox* connect_rating = NULL, * connect_position = NULL;
Label* callsign_label = NULL, * name_label = NULL, * user_label = NULL, * pass_label = NULL;
CloseButton* connect_closeb = NULL;
DisplayBox* main_chat_box = NULL, * controller_list_box = NULL, * controller_info_box = NULL, * qlc_list_box = NULL;

void DispatchOGLMessages(MSG* lpMsg);
void handleConnect();
bool handle_asel(Mirror* mirror, Aircraft* aircraft);
Aircraft* check_asel(Mirror* mirror, double x, double y);
void handleDisconnect();
bool processCommands(std::string);
void moveInterfacesOnSize();
void conn_clean();
void pull_data(InterfaceFrame& _f, CHILD_TYPE _fc);

void pop_split_line(const InterfaceFrame& frame, InputField* focusField);
ChildFrame* position_cursor_pop(InterfaceFrame& frame, InputField* input_field);

void wrap_and_clip(InputField* focusField);

void open_chat(std::string call_sign);

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

	//std::cout << TextToBinaryString("07/10/21") << std::endl;

	/*if (!(bin == "0011000000110111001011110011000000111001001011110011001000110001"
		|| bin == "0011000000110111001011110011000100110000001011110011001000110001")) {
		MessageBox(hWnd, L"Time Expired! Please contact the developer for an Extension", L"Notice",
			MB_OK | MB_ICONINFORMATION);
		return 0;
	}*/

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
		if (messages.message == WM_LBUTTONDOWN
			|| messages.message == WM_RBUTTONDOWN
			|| messages.message == WM_LBUTTONUP
			|| messages.message == WM_MOUSEMOVE
			|| messages.message == WM_RBUTTONDBLCLK)
		{
			auto it = std::find(message_queue.begin(), message_queue.end(), nullptr);
			if (it != message_queue.end())
				*it = &messages;
		}
	}

	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}

void DispatchOGLMessages(MSG* lpMsg)
{

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
		default:;
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
	if (UIManager::Instance().IsInitialized()) { // Only process if UIManager is ready
		UIManager::Instance().ProcessSystemEvent(hwnd, message, wParam, lParam);
		// If ProcessSystemEvent sets a flag indicating it handled the event,
		// you might choose to return 0 immediately for certain messages.
		// For example:
		// if (UIManager::Instance().LastEventHandled()) {
		//     switch (message) {
		//         case WM_LBUTTONDOWN:
		//         case WM_LBUTTONUP:
		//         // ... other messages fully consumed by UI ...
		//             return 0; // Event handled by UI, no further processing needed by DefWindowProc
		//     }
		// }
	}

	switch (message)                  /* handle the messages */
	{
	case WM_APP_REFRESH_CONTROLLER_LIST:
	{
		if (UIManager::Instance().IsInitialized()) {
			ControllerListWindow* ctrlListWin = dynamic_cast<ControllerListWindow*>(
				UIManager::Instance().GetWindowById("ControllerListWindowInstance")
				);
			if (ctrlListWin && ctrlListWin->visible) {
				// Call the new public notification method
				ctrlListWin->NotifyDataMightHaveChanged();
			}
		}
		return 0;
	}
	case WM_CREATE: {
		HMENU hMenuBar = CreateMenu();
		hFile = CreateMenu();
		hSettings = CreateMenu();
		HMENU hHelp = CreateMenu();

		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSettings, L"&Settings");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

		AppendMenu(hFile, MF_STRING, ID_FILE_CONNECT, L"&Connect to Sever...");
		AppendMenu(hFile, MF_STRING, ID_FILE_DISCONNECT, L"&Disconnect...");
		AppendMenu(hFile, MF_STRING, ID_FILE_OPEN, L"&Open ADX File...");
		AppendMenu(hFile, MF_STRING, ID_FILE_OPENC, L"&Open Collision File...");
		AppendMenu(hFile, MF_STRING, ID_FILE_SAVE, L"&Save Profile");
		AppendMenu(hFile, MF_STRING, ID_FILE_LOAD, L"&Load Profile");
		AppendMenu(hFile, MF_STRING, ID_FILE_EXIT, L"&Exit");
		AppendMenu(hSettings, MF_STRING, ID_SETTINGS_DEPARTS, L"&Show/Hide Departures...");
		AppendMenu(hSettings, MF_STRING, ID_SETTINGS_SQUAWKS, L"&Show/Hide All Squawk Codes...");
		AppendMenu(hSettings, MF_STRING, ID_SETTINGS, L"&Settings...");
		AppendMenu(hSettings, MF_STRING, ID_SETTINGS_CLIST, L"&Controller List...");
		AppendMenu(hSettings, MF_STRING, ID_SETTINGS_COMMS, L"&Communications Panel...");
		AppendMenu(hHelp, MF_STRING, ID_HELP_ABOUT, L"&About...");

		SetMenu(hwnd, hMenuBar);

		EnableMenuItem(hFile, ID_FILE_DISCONNECT, MF_DISABLED);

		//opengl stuff

		hWnd = hwnd;

		// Initialize the thread pool
		ThreadPool threadPool(NUM_THREADS);

		show_departures ? CheckMenuItem(hSettings, ID_SETTINGS_DEPARTS, MF_CHECKED) : CheckMenuItem(hSettings, ID_SETTINGS_DEPARTS, MF_UNCHECKED);
		show_squawks ? CheckMenuItem(hSettings, ID_SETTINGS_SQUAWKS, MF_CHECKED) : CheckMenuItem(hSettings, ID_SETTINGS_SQUAWKS, MF_UNCHECKED);

		if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
			CAPS = true;
		else
			CAPS = false;

		CreateThread(NULL, 0, OpenGLThread, hwnd, 0, 0);
		CreateThread(NULL, 0, EventThread1, hwnd, 0, NULL);
		CreateThread(NULL, 0, CalcThread1, hwnd, 0, NULL);
		userStorage1.resize(max_user_size);

		logic.push_back("08R");
		logic.push_back("09");

#ifdef _DEBUG
		loadVisualPoints(filerdr.visualization);
#endif

		/*Aircraft* cur = new Aircraft("AAL2", 0, 0);
		if (cur != NULL) {
			cur->lock();
			cur->setHeavy(true);
			cur->setLatitude(25.800704);
			cur->setLongitude(-80.300770);
			cur->setSpeed(0.0);
			cur->setHeading(85.0);
			cur->setUpdateFlag(ACF_CALLSIGN, true);
			cur->setMode(1);
			acf_map[cur->getCallsign()] = cur;
			cur->unlock();

			cur->setSquawkCode(std::to_string(random(2000, 5200)));

			FlightPlan& fp = *cur->getFlightPlan();
			fp.departure = "KMIA";
			fp.arrival = "KSLC";
			fp.route = "HURCN3 SMELZ Q116 JAWJA DEFUN MERDN IZAAC LIT KM33G KK42C KK45A ZAROS BRK EKR LEEHY5";
			fp.remarks = "/v/";
			++fp.cycle;
		}
		userStorage1[0] = dynamic_cast<User*>(cur);


		Aircraft* cur2 = new Aircraft("EGF4427", 0, 0);
		if (cur2 != NULL) {
			cur2->lock();
			cur2->setHeavy(false);
			cur2->setLatitude(25.801866);
			cur2->setLongitude(-80.285309);
			cur2->setSpeed(0.0);
			cur2->setHeading(190.0);
			cur2->setUpdateFlag(ACF_CALLSIGN, true);
			cur2->setMode(1);
			acf_map[cur2->getCallsign()] = cur2;
			cur2->unlock();

			cur2->setSquawkCode(std::to_string(random(2000, 5200)));
		}
		userStorage1[1] = dynamic_cast<User*>(cur2);*/

		break;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		int msg_size = wParam;

		if (msg_size == SIZE_MINIMIZED) {
			// Window is minimized, so skip the resizing logic
			break;
		}

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

		if (msg_size != SIZE_MINIMIZED && msg_size != SIZE_MAXIMIZED)
		{
			moveInterfacesOnSize();
		}

		if (zoom_phase >= 2)
			mZoom = zoom_from_range();

		updateFlags[GBL_COLLISION_LINE] = true;
		updateFlags[GBL_CALLSIGN] = true;
		updateFlags[GBL_VECTOR] = true;

		resize = true;
		rendererFlags["buttons"] = true;
		rendererFlags["legend"] = true;
		rendererFlags["renderInterfaces"] = true;
		rendererFlags["drawings"] = true;
		rendererFlags["renderConf"] = true;
		rendererFlags["renderDate"] = true;
		rendererFlags["renderCoordinates"] = true;
		rendererFlags["renderDepartures"] = true;
		RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		rendererFlags["renderAllInputText"] = true;
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

			LONG dx = dragged_mir->cur_pt->x - dragged_mir->s_pt->x;
			LONG dy = dragged_mir->cur_pt->y - dragged_mir->s_pt->y;

			dragged_mir->handleMovement(dragged_mir->startX + dx, dragged_mir->startY + -dy);
		}
		else if (dragged_pos)
		{
			WORD x = LOWORD(lParam), y = (CLIENT_HEIGHT - HIWORD(lParam));
			//double coords[3];
			//GetOGLPos(x, HIWORD(lParam), coords);
			//dragged_pos->setLat(dragged_pos->getLat() + (coords[0] - MOUSE_POS->y_));
			//dragged_pos->setLon(dragged_pos->getLon() + (coords[1] - MOUSE_POS->x_));
		}
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_CONNECT:
		{
			//handleConnect();
			ConnectWindow* connectWin = UIManager::ShowOrCreateInstance<ConnectWindow>("ConnectWindowInstance", _WINPOS_CONNECT);
			if (!connectWin && !UIManager::Instance().IsInitialized()) {
				MessageBox(hwnd, L"UI System not ready.", L"Error", MB_OK | MB_ICONERROR);
			}
			break;
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
			show_departures ? CheckMenuItem(hSettings, ID_SETTINGS_DEPARTS, MF_CHECKED) : CheckMenuItem(hSettings, ID_SETTINGS_DEPARTS, MF_UNCHECKED);
		}
		break;
		case ID_SETTINGS_SQUAWKS:
		{
			show_squawks = !show_squawks;
			show_squawks ? CheckMenuItem(hSettings, ID_SETTINGS_SQUAWKS, MF_CHECKED) : CheckMenuItem(hSettings, ID_SETTINGS_SQUAWKS, MF_UNCHECKED);
			updateFlags[GBL_CALLSIGN] = true;

		}
		break;
		case ID_SETTINGS: // Replace ID_SETTINGS with the actual menu ID you defined for settings
		{
			SettingsWindow* settingsWin = UIManager::ShowOrCreateInstance<SettingsWindow>("SettingsWindowInstance"); // No specific _WINPOS_ for this one yet
			if (!settingsWin && !UIManager::Instance().IsInitialized()) {
				MessageBox(hwnd, L"UI System not ready.", L"Error", MB_OK | MB_ICONERROR);
			}
			break;
		}
		case ID_SETTINGS_CLIST:
		{
			ControllerListWindow* ctrlListWin = UIManager::ShowOrCreateInstance<ControllerListWindow>(
				"ControllerListWindowInstance",
				_WINPOS_CTRLLIST // Make sure this constant is defined and used for saving positions
			);
			if (ctrlListWin) {
				// The window is now shown or created.
				// The actual content update will happen via refresh_ctrl_list -> UpdateDisplay.
				// You might trigger an initial refresh here if needed.
				// refresh_ctrl_list(); // This will find the window and update it
			}
			else if (!UIManager::Instance().IsInitialized()) {
				MessageBox(hwnd, L"UI System not ready.", L"Error", MB_OK | MB_ICONERROR);
			}
			break;
		}
		case ID_SETTINGS_COMMS:
		{
			CommsWindowWidget* commsWin = UIManager::ShowOrCreateInstance<CommsWindowWidget>("CommsWindowInstance", _WINPOS_COMMS);
			if (!commsWin && !UIManager::Instance().IsInitialized()) {
				MessageBox(hwnd, L"UI System not ready.", L"Error", MB_OK | MB_ICONERROR);
			}
			break;
		}
		//case ID_SETTINGS_COMMS:
		//{
		//	if (communications == NULL) {
		//		//TODO save X and Y positions when moved
		//		int* wdata = USER->userdata.window_positions[_WINPOS_COMMS];
		//		RenderCommunications(true, wdata[0], wdata[1], 0);
		//	}
		//	else if (!communications->render)
		//	{
		//		communications->doOpen(true, true);
		//	}
		//}
		break;
		case ID_FILE_OPEN:
		case ID_FILE_OPENC:
		{
			OPENFILENAME ofn;
			TCHAR szFileName[MAX_PATH] = L"";

			ZeroMemory(&ofn, sizeof(ofn));

			auto wideParam = LOWORD(wParam);

			ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = (wideParam == ID_FILE_OPENC) ? L"Collision Path Files (*.cpf)\0*.cpf\0All Files (*.*)\0*.*\0" :
				L"ASDE-X Files (*.adx)\0*.adx\0All Files (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = (wideParam == ID_FILE_OPENC) ? L"cpf" : L"adx";

			if (GetOpenFileName(&ofn))
			{
				std::wstring wide(szFileName);
				std::string final1 = ws2s(wide);
				if (wParam == ID_FILE_OPEN)
				{
					sendSystemMessage("ADX File Loaded.");
					open_adx(final1);
				}
				else
				{
					parseCpfFile(final1, filerdr.collisionPaths);
					sendSystemMessage("Collision Paths Loaded.");
				}
			}
		}
		break;
		case ID_FILE_SAVE:
			save_info();
			break;
		case ID_FILE_LOAD:
			read_info();
			break;
		case ID_FILE_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case ID_HELP_ABOUT:
		{
			int ret = DialogBox(GetModuleHandle(NULL),
				MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)AboutDlgProc);
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
	case WM_RBUTTONDOWN:
	{
		WORD x = LOWORD(lParam), y = (CLIENT_HEIGHT - HIWORD(lParam));

	}
	break;
	case WM_LBUTTONDOWN:
	{
		WORD x = LOWORD(lParam), y = (CLIENT_HEIGHT - HIWORD(lParam));
		bool clicked_interface = false;
		for (auto it = rendered_frames.rbegin(); it != rendered_frames.rend(); ++it) {
			InterfaceFrame* frame = *it;
			if (frame && frame->render) {
				ChildFrame* clicked1 = nullptr;
				for (auto child = frame->children.rbegin(); child != frame->children.rend(); ++child) {
					if (ChildFrame* children = *child) {
						BasicInterface* inter2 = children->border;
						if (children->handleClick(clicked1, x, y))
							break;
						if (!clicked1 && inter2->isBounds()) {
							double vertx[4] = { inter2->getStartX(), inter2->getStartX(), inter2->getEndX(), inter2->getEndX() };
							double verty[4] = { inter2->getStartY(), inter2->getEndY(), inter2->getEndY(), inter2->getStartY() };
							bool clicked = pnpoly(4, vertx, verty, x, y);
							if (clicked) {
								clicked1 = children;
								break;
							}
						}
					}
				}
				if (clicked1) {
					if (clicked1->type == CHILD_TYPE::INPUT_FIELD && !dynamic_cast<InputField*>(clicked1)->editable) {
						//dont do anything with non editable field
					}
					else
					{
						if (focusChild != clicked1) {
							clicked1->setFocus();
						}
						if (focusChild == clicked1) {
							clicked1->doAction();
							if (clicked1->type == CHILD_TYPE::INPUT_FIELD)
							{
								auto* field = dynamic_cast<InputField*>(clicked1);
								field->calcCursorPos(x, y);
								RenderChild(field, CHILD_TYPE::INPUT_FIELD);
							}
						}
					}
					clicked_interface = true;
					break;
				}

				BasicInterface* clicked2 = nullptr, * inter1 = frame->border;
				if (inter1 && inter1->isBounds() && frame->pannable) {
					int b_offset_Y = 25;
					double vert_x[4] = { inter1->getStartX(), inter1->getStartX(), inter1->getEndX(), inter1->getEndX() };
					double vert_y[4] = { inter1->getEndY() - b_offset_Y, inter1->getEndY(), inter1->getEndY(), inter1->getEndY() - b_offset_Y, };
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
					frame->s_pt->x = static_cast<int>(static_cast<short>(LOWORD(lParam)));
					frame->s_pt->y = static_cast<int>(static_cast<short>(HIWORD(lParam)));
					dragged = frame;
					dragged_bounds = clicked2;
					clicked_interface = true;
					break;
				}
			}
		}
		if (!clicked_interface)
		{
			TopButton* clicked_tbutton = nullptr;
			for (auto btn = BUTTONS.rbegin(); btn != BUTTONS.rend(); ++btn)
			{
				TopButton* curButton = *btn;
				int* params = curButton->getParams();
				double vertx[4] = { params[0], params[0], params[2], params[2] };
				double verty[4] = { params[1], params[3], params[3], params[1] };
				if (bool clicked = pnpoly(4, vertx, verty, x, y)) {
					if (curButton->handle())
					{
						curButton->on = !curButton->on;
						rendererFlags["buttons"] = true;
					}
					clicked_tbutton = curButton;
					clicked_interface = true;
					break;
				}
			}
		}
		if (!clicked_interface)
		{
			bool clicked_mirror = false;
			//check for mirrors
			for (auto it3 = mirrors.rbegin(); it3 != mirrors.rend(); ++it3)
			{
				Mirror* mir = *it3;
				if (mir)
				{
					Mirror& mirror = *mir;
					int b_offset_Y = 25;
					double end_x = mirror.getX() + mirror.getWidth(), end_y = mirror.getY() + mirror.getHeight();
					double vert_x[4] = { mirror.getX(), mirror.getX(), end_x, end_x };
					double vert_y[4] = { end_y - b_offset_Y, end_y, end_y, end_y - b_offset_Y, };
					bool clicked_bar = pnpoly(4, vert_x, vert_y, x, y);
					if (clicked_bar) {
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
						clicked_mirror = true;
						break;
					}
					else
					{
						//clicked anywhere else in the mirrror
						double end_x = mirror.getX() + mirror.getWidth(), end_y = mirror.getY() + mirror.getHeight();
						double vert_x[4] = { mirror.getX(), mirror.getX(), end_x, end_x };
						double vert_y[4] = { mirror.getY(), end_y, end_y, mirror.getY(), };
						bool clicked = pnpoly(4, vert_x, vert_y, x, y);
						if (clicked)
						{
							clicked_mirror = true;
							Aircraft* asel = check_asel(mir, x, y);
							if (asel)
							{
								handle_asel(mir, asel);
							}
							else
							{
								dragged_pos = mir;
							}
							break;
						}
					}
				}
			}
			if (!clicked_mirror)
			{
				Aircraft* asel = check_asel(nullptr, x, y);
				if (asel)
				{
					handle_asel(nullptr, asel);
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

				USER->WindowMove(dragged, dragged->border->getPosX(), dragged->border->getPosY());
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
		else if (dragged_pos)
		{
			dragged_pos = nullptr;
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
			double vertx[4] = { params[0], params[0], params[2], params[2] };
			double verty[4] = { params[1], params[3], params[3], params[1] };
			bool clicked = pnpoly(4, vertx, verty, pt.x, pt.y);
			if (clicked) {
				if (curButton->handleScroll(val > 0))
				{
					rendererFlags["buttons"] = true;
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
					BasicInterface& bdr = *focus->border;
					if (bdr.isBounds()) {
						if (val < 0)
							dynamic_cast<DisplayBox*>(focus)->doActionDown();
						else if (val > 0)
							dynamic_cast<DisplayBox*>(focus)->doActionUp();
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
							if (ChildFrame* children = *child) {
								BasicInterface& border = *children->border;
								if (border.isBounds()) {
									double vertx[4] = { border.getStartX(), border.getStartX(), border.getEndX(), border.getEndX() };
									double verty[4] = { border.getStartY(), border.getEndY(), border.getEndY(), border.getStartY() };
									bool clicked = pnpoly(4, vertx, verty, pt.x, pt.y);
									if (clicked) {
										if (val < 0) {
											dynamic_cast<DisplayBox*>(children)->doActionDown();
											break;
										}
										else if (val > 0) {
											dynamic_cast<DisplayBox*>(children)->doActionUp();
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
			if (focusChild) {
				CHILD_TYPE type = focusChild->type;
				if (type == CHILD_TYPE::INPUT_FIELD) {
					InputField& focusField = (InputField&)*focusChild;
					if (focusField.input.size() > 0)
					{
						focusField.clearInput();
						focusField.setCursor();
						RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
						focusField.history_index = (focusField.history.size() - 1);
					}
					else
					{
						ASEL = nullptr;
						rendererFlags["renderConf"] = true;
					}
				}
				else
				{
					ASEL = nullptr;
					rendererFlags["renderConf"] = true;
				}
			}
		}
		else if (wParam == VK_F1) {
		}
		else if (wParam == VK_F2) {
			const char* cmd = ".AN ";
			pass_command(cmd);
		}
		else if (wParam == VK_F3) {
		}
		else if (wParam == VK_F4) {
		}
		else if (wParam == VK_F5) {
		}
		else if (wParam == VK_F6) {
			const char* cmd = ".SS ";
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
		else if (wParam == VK_TAB) {
			if (focusChild) {
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
			CONTROL = true;
		}
		else if (wParam == VK_LCONTROL) {
			CONTROL = true;
		}
		else if (wParam == VK_RCONTROL) {
			CONTROL = true;
		}
		else if (wParam == 0xFF) {//FN Key

		}
		else if (wParam == VK_ADD)
		{
			if (ASEL && ASEL->getIdentity()->type == CLIENT_TYPES::PILOT_CLIENT)
			{
				Aircraft* aircraft = (Aircraft*)ASEL;
				if (focusChild == main_chat_input && boost::istarts_with(main_chat_input->input, "."))
				{
					if (processCommands(main_chat_input->input + aircraft->getCallsign()))
					{
						main_chat_input->clearInput();
						main_chat_input->setCursor();
						RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
					}
				}
				else if (focusChild == terminal_input && boost::istarts_with(terminal_input->input, "."))
				{
					if (processCommands(terminal_input->input + aircraft->getCallsign()))
					{
						terminal_input->clearInput();
						terminal_input->setCursor();
						RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
					}
				}
			}
			else
			{
				//look for aircraft
			}
		}
		else if (wParam == VK_RETURN)
		{
			if (focusChild) {
				CHILD_TYPE type = focusChild->type;
				if (type == CHILD_TYPE::INPUT_FIELD)
				{
					auto* focusField = dynamic_cast<InputField*>(focusChild);
					InterfaceFrame& frame = *focusField->getFrame();
					int frame_id = frame.id;
					if (focusField->line_ptr)
					{
						focusField->handle_box();
						pull_data(frame, type);
						main_chat_input->setFocus();
					}
					else if (frame_id == TERMINAL_COMMAND && focusField == terminal_input)
					{// terminal commands
						if (!focusField->input.empty()) {
							if (processCommands(focusField->input)) {
								focusField->clearInput();
								focusField->setCursor();
								RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
							}
							else
							{
								focusField->clearInput();
								focusField->setCursor();
								RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
							}
						}
					}
					else if (frame_id == MAIN_CHAT_INTERFACE && focusField == main_chat_input)
					{// main chat
						if (!focusField->input.empty())
						{
							focusField->history.emplace_back(focusField->input);
							focusField->history_index = focusField->history.size() - 1;
							if (processCommands(focusField->input)) {
								focusField->clearInput();
								focusField->setCursor();
								RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
							}
							else
							{
								sendMainChatMessage(focusField);
							}
						}
					}
					else if (is_privateinterface(frame_id))
					{
						if (focusField->index == PRIVATE_MESSAGE_INPUT)
						{
							if (!focusField->input.empty())
							{
								focusField->history.push_back(focusField->input);
								focusField->history_index = focusField->history.size() - 1;
								if (processCommands(focusField->input))
								{
									focusField->clearInput();
									focusField->setCursor();
									RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
								}
								else
								{
									sendPrivateChatMessage(frame, focusField);
								}
							}
						}
					}
					else
					{
						//handle regular input field
						focusField->history.push_back(focusField->input);
						focusField->history_index = focusField->history.size() - 1;
						focusField->handle_entry();
						main_chat_input->setFocus();
					}
				}
			}
		}
		else if (wParam == VK_LEFT) {
			if (focusChild) {
				CHILD_TYPE type = focusChild->type;
				if (type == CHILD_TYPE::COMBO_BOX) {
					ComboBox* box = (ComboBox*)focusChild;
					if (box->pos > 0) {
						box->pos--;
						rendererFlags["drawings"] = true;
					}
				}
				else if (type == CHILD_TYPE::INPUT_FIELD) {
					InputField* input = (InputField*)focusChild;
					input->cursorLeft();
				}
			}
		}
		else if (wParam == VK_RIGHT) {
			if (focusChild) {
				CHILD_TYPE type = focusChild->type;
				if (type == CHILD_TYPE::COMBO_BOX) {
					ComboBox* box = (ComboBox*)focusChild;
					if (box->pos < (box->options.size() - 1)) {
						box->pos++;
						rendererFlags["drawings"] = true;
					}
				}
				else if (type == CHILD_TYPE::INPUT_FIELD) {
					InputField* input = (InputField*)focusChild;
					input->cursorRight();
				}
			}
		}
		else if (wParam == VK_UP) {
			if (focusChild && focusChild->type == CHILD_TYPE::INPUT_FIELD)
			{
				InputField* focusField = (InputField*)focusChild;

				if (focusField == main_chat_input)
				{
					if (focusField->history_index >= 0 && focusField->history_index < focusField->history.size())
					{
						std::string line = focusField->history[focusField->history_index];
						if (!line.empty())
						{
							focusField->clearInput();
							focusField->input = line;
							focusField->setCursorAtEnd();
							RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
						}
					}
					if ((focusField->history_index - 1) >= 0)
						focusField->history_index--;
				}
			}
		}
		else if (wParam == VK_DOWN) {
			if (focusChild && focusChild->type == CHILD_TYPE::INPUT_FIELD)
			{
				InputField* focusField = dynamic_cast<InputField*>(focusChild);

				if (focusField == main_chat_input)
				{
					if (focusField->history_index >= 0 && focusField->history_index < focusField->history.size())
					{
						std::string line = focusField->history[focusField->history_index];
						if (!line.empty())
						{
							focusField->clearInput();
							focusField->input = line;
							focusField->setCursorAtEnd();
							RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
						}
					}
					if ((focusField->history_index + 1) < focusField->history.size())
						focusField->history_index++;
				}
			}
		}
		else if (wParam == VK_BACK) {
			if (focusChild && focusChild->type == CHILD_TYPE::INPUT_FIELD) {
				auto* focusField = dynamic_cast<InputField*>(focusChild);
				InterfaceFrame& frame = *focusField->getFrame();
				if (focusField->editable) {
					if (!focusField->input.empty()) {
						bool popped = focusField->popInput();
						focusField->setCursor();
						// Optional: Handle splitting if needed
						pop_split_line(frame, focusField);
						RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
						focusField->history_index = 0;
					} else if (focusField->cursor_pos == 0) {
						if (position_cursor_pop(frame, focusField)) {
							// Successfully merged with the previous line
							// Rendered inside position_cursor_pop
						}
						else {
							// At the first line, do nothing or provide feedback
						}
					}
				}
			}
		}
		else if (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0) {
			if (GetKeyState(VK_MENU) < 0) {
				switch (wParam)
				{
				case 'V':
				{
					SHOW_POINTS = !SHOW_POINTS;
					if (SHOW_POINTS) {
						rendererFlags["pointvis"] = true;
					}
					break;
				}
				}
			}
			else {
				switch (wParam)
				{
				case 'D':
				{
					// SHIFT+D is pressed
					DUMP_COLLISION = !DUMP_COLLISION;
					std::wstring msg = DUMP_COLLISION ? L"Dumping Collision Paths" : L"No Longer dumping Collision Paths";
					MessageBox(hwnd, msg.c_str(), L"Key Pressed", MB_OK | MB_ICONINFORMATION);
					if (DUMP_COLLISION) {
						sendSystemMessage("Dumping Collision Points.");
					}
					else {
						for (auto& latlon : debug_vis.getPoints())
							filerdr.clickPoints.push_back(Point2(latlon.lat, latlon.lat));
						filerdr.DumpCollisionsToFile();
						sendSystemMessage("Finished Collision Points.");
						debug_vis.clear();
						rendererFlags["renderLineVis"] = true;
					}
					break;
				}
				case 'C':
				{
					DUMP_CLOSURE = !DUMP_CLOSURE;
					std::wstring msg = DUMP_CLOSURE ? L"Dumping Closure Points" : L"No Longer dumping Closure Points";
					MessageBox(hwnd, msg.c_str(), L"Key Pressed", MB_OK | MB_ICONINFORMATION);
					if (DUMP_CLOSURE) {
						sendSystemMessage("Dumping Closure Points.");
					}
					else {
						rendererFlags["redrawClosures"] = true;
						sendSystemMessage("Finished Closure Points.");
						sendTempData(debug_vis.getPoints());
						debug_vis.clear();
						rendererFlags["renderLineVis"] = true;
					}
					break;
				}
				case 'Z':
				{
					if (DUMP_CLOSURE || DUMP_COLLISION) {
						debug_vis.pop_back();
						rendererFlags["renderLineVis"] = true;
					}
					break;
				}
				}
			}
		}
		else {
			char c = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
			if (focusChild && focusChild->type == CHILD_TYPE::INPUT_FIELD) {
				InputField& focusField = *dynamic_cast<InputField*>(focusChild);
				InterfaceFrame& frame = *focusField.getFrame();
				if (focusField.editable)
				{
					if (frame.interfaces[FRAME_BOUNDS])
					{
						if (CONTROL && (c == 'v' || c == 'V'))
						{
							std::string clip = GetClipboardText();
							for (char& s : clip)
							{
								if (!focusField.can_type())
									break;
								focusField.pushInput(false, s);
								focusField.setCursor();
								wrap_and_clip(dynamic_cast<InputField*>(focusChild));
								RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
								focusField.history_index = focusField.history.size() - 1;
							}
						}
						else
						{
							BYTE keyboardState[256];
							BOOL ks = GetKeyboardState(keyboardState);
							WORD ascii;
							int len = ::ToAscii(wParam, (lParam >> 16) & 0xFF, keyboardState, &ascii, 0);
							if (len == 1) {
								if (focusField.can_type()) {
									focusField.pushInput(false, ascii);
									focusField.setCursor();
									wrap_and_clip(dynamic_cast<InputField*>(focusChild));
									RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
									focusField.history_index = focusField.history.size() - 1;
								}
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
		switch (wParam)
		{
		case VK_CAPITAL:
		{
			if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
				CAPS = true;
			else
				CAPS = false;
			break;
		}
		case 0x11: case VK_LCONTROL: case VK_RCONTROL:
		{
			CONTROL = false;
			break;
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
		sendErrorMessage(interface_error);
		return;
	}
	if (connected) {
		return;
	}
	if (connectFrame == NULL) {
		//TODO save X and Y positions when moved
		int* wdata = USER->userdata.window_positions[_WINPOS_CONNECT];
		RenderConnect(wdata[0], wdata[1]);
		connect_callsign->setFocus();
	}
	else {
		if (!connectFrame->render) {
			connectFrame->doOpen(false, true);
			connect_callsign->setFocus();
		}
	}
}

bool handle_asel(Mirror* mirror, Aircraft* aircraft)
{
	if (focusChild == main_chat_input && boost::istarts_with(main_chat_input->input, "."))
	{
		if (processCommands(main_chat_input->input + aircraft->getCallsign()))
		{
			main_chat_input->clearInput();
			main_chat_input->setCursor();
			RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		}
	}
	else if (focusChild == terminal_input && boost::istarts_with(terminal_input->input, "."))
	{
		if (processCommands(terminal_input->input + aircraft->getCallsign()))
		{
			terminal_input->clearInput();
			terminal_input->setCursor();
			RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
		}
	}
	ASEL = aircraft;
	rendererFlags["renderConf"] = true;
	return true;
}

Aircraft* check_asel(Mirror* mirror, double x, double y)
{
	bool is_mirror = mirror ? true : false;
	if (acf_map.size() > 0)
	{
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++)
		{
			// iterator->first = key
			Aircraft* acf_ptr = iter->second;
			if (acf_ptr != nullptr)
			{
				Aircraft& aircraft = *acf_ptr;
				aircraft.lock();
				bool heavy = aircraft.isHeavy();
				bool standby = aircraft.getMode() == 0 ? true : false;
				double a_size = get_asize(heavy, standby);
				aircraft.unlock();

				double maxX = standby ? unknownBlip->getMaxX() : aircraftBlip->getMaxX();
				double maxY = standby ? unknownBlip->getMaxX() : aircraftBlip->getMaxY();

				double offset = (maxX > maxY ? maxX : maxY) * a_size;

				double _ax = is_mirror ? mirror->wndc[&aircraft][1] : aircraft.wnd_loc[1],
					_ay = is_mirror ? mirror->wndc[&aircraft][0] : aircraft.wnd_loc[0];
				double _dist = plain_dist(_ax, _ay, x, y);

				if (_dist <= offset)
					return acf_ptr;
			}
		}
	}
	return nullptr;
}

void handleDisconnect() {
	if (!connected) {
		return;
	}
	send_disconnect();
}

bool processCommands(std::string command)
{
	if (boost::istarts_with(command, ".SS")) { // Assuming 'command' is the string from input
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			int* wdata = USER->userdata.window_positions[_WINPOS_FLIGHTPLAN]; // For potential default positioning
			capitalize(array3[1]);
			std::string call_sign_str = array3[1];
			std::string fpWindowId = "FPWindow_" + call_sign_str;

			WindowWidget* existingFpWin_widget = UIManager::Instance().GetWindowById(fpWindowId);
			FlightPlanWindow* existingFpWin = dynamic_cast<FlightPlanWindow*>(existingFpWin_widget);

			if (existingFpWin) {
				UIManager::Instance().BringWindowToFront(existingFpWin);
				// If aircraft data might have changed externally, tell the window to update
				auto it_acf = acf_map.find(call_sign_str);
				if (it_acf != acf_map.end()) {
					existingFpWin->UpdateData(it_acf->second); // Refresh data
				}
				existingFpWin->RequestFocus(); // Or focus a specific field if desired
			}
			else {
				// Window doesn't exist, create a new one
				std::unique_ptr<FlightPlanWindow> fpWin_uptr;
				Aircraft* target_aircraft = nullptr;

				auto it_acf = acf_map.find(call_sign_str);
				if (it_acf != acf_map.end()) {
					target_aircraft = it_acf->second;
					if (target_aircraft && target_aircraft->getIdentity() && target_aircraft->getIdentity()->type == CLIENT_TYPES::PILOT_CLIENT) {
						// Aircraft known, has flight plan data or needs one requested
						sendFlightPlanRequest(*target_aircraft); // Request latest data if needed
						fpWin_uptr = std::make_unique<FlightPlanWindow>(target_aircraft);
					}
					else {
						// Aircraft found but not a pilot type, or some other issue.
						// Treat as new/unknown for now, or show error.
						// For now, let's assume we open a blank FP window for the callsign.
						fpWin_uptr = std::make_unique<FlightPlanWindow>(call_sign_str);
					}
				}
				else {
					// Callsign completely unknown to acf_map, open a "new" flight plan window
					fpWin_uptr = std::make_unique<FlightPlanWindow>(call_sign_str);
				}

				if (fpWin_uptr) {
					fpWin_uptr->id = fpWindowId;

					Size prefSize = fpWin_uptr->Measure();
					Rect initialBounds;

					// Use saved position if valid, otherwise center
					if (wdata[0] != -1 && wdata[1] != -1) {
						initialBounds = { wdata[0], wdata[1], prefSize.width, prefSize.height };
					}
					else {
						initialBounds = { (CLIENT_WIDTH - prefSize.width) / 2,
										 (CLIENT_HEIGHT - prefSize.height) / 2,
										 prefSize.width, prefSize.height };
					}

					// Clamp to screen boundaries
					initialBounds.width = std::min(initialBounds.width, CLIENT_WIDTH);
					initialBounds.height = std::min(initialBounds.height, CLIENT_HEIGHT);
					initialBounds.x = std::max(0, std::min(initialBounds.x, CLIENT_WIDTH - initialBounds.width));
					initialBounds.y = std::max(0, std::min(initialBounds.y, CLIENT_HEIGHT - initialBounds.height));

					fpWin_uptr->Arrange(initialBounds);
					UIManager::Instance().AddWindow(std::move(fpWin_uptr));

					WindowWidget* newWin = UIManager::Instance().GetWindowById(fpWindowId);
					if (newWin) newWin->RequestFocus();
				}
			}
		}
		return true; // Command was processed (or attempted)
	}
	if (boost::istarts_with(command, ".AN")) {
		return true;
	}
	if (boost::istarts_with(command, ".CHAT")) {
		std::vector<std::string> array3 = split(command, " ");
		if (array3.size() == 2) {
			capitalize(array3[1]);
			std::string targetCallsign = array3[1];

			if (USER && USER->getIdentity() && USER->getIdentity()->callsign == targetCallsign) {
				sendErrorMessage("Cannot open private chat with yourself.");
				return true;
			}

			std::string windowId = "PrivChat_" + targetCallsign;

			PrivateChatWindow* pmWin = UIManager::ShowOrCreateInstance<PrivateChatWindow>(
				windowId,
				-1, // No specific _WINPOS_ constant for PMs, will center or use default
				targetCallsign // Argument for PrivateChatWindow constructor
			);

			if (!pmWin && !UIManager::Instance().IsInitialized()) {
				MessageBox(hWnd, L"UI System not ready.", L"Error", MB_OK | MB_ICONERROR);
			}
			else if (pmWin) {
				// Window is shown/created, focus should be handled by ShowOrCreateInstance or constructor
			}
		}
		return true;
	}
	if (boost::istarts_with(command, ".OM")) {
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
				addCollisionsToMirror(mir);
			}
		}
		return true;
	}
	if (boost::istarts_with(command, ".CM")) {
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
	if (boost::istarts_with(command, ".test")) {
		controller_list_box->addLineTop("------------OBSERVER----------", CHAT_TYPE::MAIN);
		rendererFlags["drawings"] = true;
		return true;
	}
	if (boost::istarts_with(command, "/")) {
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
	//34.142.27.168
	const std::string ip = "127.0.0.1";
	tcpinterface& tcp = *intter;
	if (tcp.connectNew(hWnd, ip, 4403)) {
		connected = true;
		tcp.startT(hWnd);
		Identity& id = *USER->getIdentity();
		sendSystemMessage("Connected.");
		EnableMenuItem(hFile, ID_FILE_CONNECT, MF_DISABLED);
		EnableMenuItem(hFile, ID_FILE_DISCONNECT, MF_ENABLED);
		BasicStream stream = BasicStream(512);
		CLIENT_TYPES type = id.type;
		tcp.hand_shake = true;
		tcp.current_op = 45;
		stream.create_frame_var_size_word(45);
		stream.write_int(PROTO_VERSION);
		stream.write_string(USER->getCallsign().c_str());
		stream.write_string(id.login_name.c_str());
		stream.write_string(id.username.c_str());
		stream.write_string(id.password.c_str());
		stream.write_qword(1000);//request time
		stream.write_qword(doubleToRawBits(USER->getLatitude()));
		stream.write_qword(doubleToRawBits(USER->getLongitude()));
		stream.write_short(USER->getVisibility());
		stream.write_byte(static_cast<int>(type));
		stream.write_3byte(USER->userdata.frequency[0]);
		stream.write_3byte(USER->userdata.frequency[1]);
		if (type == CLIENT_TYPES::CONTROLLER_CLIENT)
		{
			stream.write_byte(id.controller_rating);
			stream.write_byte(static_cast<int>(id.controller_position));
		}
		else if (type == CLIENT_TYPES::PILOT_CLIENT)
		{
			stream.write_byte(0);
			stream.write_string("King Air 350");
			stream.write_string("0000");
			stream.write_byte(0 << 4 | 0);
			stream.write_qword(0);//info hash
		}
		stream.end_frame_var_size_word();
		tcp.sendMessage(&stream);
	}
}

void send_disconnect()
{
	sendDisconnect();
	closesocket(intter->sConnect);
}

void disconnect()
{
	sendSystemMessage("Disconnected.");
	if (intter->position_updates && !intter->position_updates->eAction.paused)
		intter->position_updates->toggle_pause();
	conn_clean();
	connected = false;
	EnableMenuItem(hFile, ID_FILE_CONNECT, MF_ENABLED);
	EnableMenuItem(hFile, ID_FILE_DISCONNECT, MF_DISABLED);
}

void conn_clean()
{
	clear_ctrl_list();
	controller_list_box->clearLines();
	qlc_list_box->clearLines();
	for (int i = 1; i < max_user_size; i++)
	{
		const User* user = userStorage1[i];
		if (user && user != USER)
		{
			delete user;
			userStorage1[i] = nullptr;
		}
	}
	for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it)
	{
		const Collision* collision = it->second;
		delete collision;
	}
	if (!departures.empty())
	{
		departures.clear();
		rendererFlags["renderDepartures"] = true;
	}
	Collision_Map.clear();
	controller_map.clear();
	acf_map.clear();
	users_map.clear();
	ASEL = nullptr;
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
	boost::posix_time::ptime frameStart, frameEnd;
	// Initialize frameEnd to a value that makes the first deltaTime sensible
	frameEnd = boost::posix_time::microsec_clock::local_time() - boost::posix_time::milliseconds(16); // Approx 16ms ago
	long long desiredFrameTimeMillis = 1000 / 60; // Target 60 FPS

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

	// 1. Initialize FontManager *after* GL context is current
	FontManager::Instance().Initialize(hDC);

	// 2. Load your default/existing fonts into the FontManager
	//    Match these to the parameters of your old BuildFont calls.
	//    BuildFont(LPCWSTR font_name, int font_height, bool bold, unsigned int* base, HFONT* oldfont1)

	// Old: BuildFont(L"SansSerif", -15, true, &legendBase, &legendFont);
	FontManager::Instance().LoadFont(FONT_KEY_LEGEND, L"SansSerif", -15, true, false);

	// Old: BuildFont(L"Consolas", -11, false, &topButtonBase, &topBtnFont);
	FontManager::Instance().LoadFont(FONT_KEY_TOP_BUTTON, L"Consolas", -11, false, false);

	// Old: BuildFont(L"Times New Roman", -15, true, &titleBase, &titleFont);
	FontManager::Instance().LoadFont(FONT_KEY_TITLE, L"Times New Roman", -15, true, false);

	// Old: BuildFont(L"Consolas", -13, false, &confBase, &confFont);
	FontManager::Instance().LoadFont(FONT_KEY_CONFIG, L"Consolas", -13, false, false);

	// Old: BuildFont(L"Courier New", -12, false, &callSignBase, &callSignFont);
	FontManager::Instance().LoadFont(FONT_KEY_CALLSIGN, L"Courier New", -12, false, false);

	// Old: BuildFont(L"Times New Roman", -11, true, &labelBase, &labelFont);
	FontManager::Instance().LoadFont(FONT_KEY_UI_LABEL, L"Times New Roman", -11, true, false);

	// Old: BuildFont(L"Consolas", -11, false, &errorBase, &errorFont);
	FontManager::Instance().LoadFont(FONT_KEY_ERROR, L"Consolas", -11, false, false);

	// 3. Initialize UIManager (it can now use FontManager indirectly via renderer/style)
	UIManager::Instance().Initialize(std::make_unique<OpenGLUIRenderer>(hDC), CLIENT_WIDTH, CLIENT_HEIGHT);

	LoadMainChatInterface(false);
	RenderControllerList(false, -1, -1);
	int id = 0, items_opened = 0, c_id = -1;
	while (items_opened < MAX_PMCHATS)
	{
		if (is_privateinterface(id))
		{
			LoadPrivateChat(-1, -1, "NOT_LOGGED", true, false, id);
			++items_opened;
		}
		++id;
	}
	dynamic_cast<InputField*>(frames_def[MAIN_CHAT_INTERFACE]->children[MAIN_CHAT_INPUT])->setFocus();
	updateFlags[GBL_AIRCRAFT] = true;
	std::cout.precision(10);
	ConfigUpdates position_updates = ConfigUpdates();
	position_updates.eAction.setTicks(0);
	event_manager1->addEvent(&position_updates);
	read_info();
	while (!done) {
		frameStart = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration td = frameStart - frameEnd; // Calculate delta time
		float deltaTime = static_cast<float>(td.total_milliseconds()) / 1000.0f;
		if (deltaTime <= 0.0f) deltaTime = 1.0f / 60.0f; // Prevent zero or negative dt on first frame

		// --- UI UPDATE ---
		if (UIManager::Instance().IsInitialized()) {
			UIManager::Instance().Update(deltaTime);
		}

		preFlags();

		//Draw Main Scene
		ResizeGLScene();
		DrawGLScene();

		//Draw Aircraft Data like Callsigns vectors
		ResizeDataGLScene();
		DrawData();

		//Draw Sub Scenes
		for (Mirror* mirror : mirrors) {
			if (mirror)
			{
				ResizeMirrorGLScene(*mirror);
				DrawMirrorScenes(*mirror);
			}
		}

		//Draw Subscene Aircraft Data like Callsigns vectors
		for (Mirror* mirror : mirrors) {
			if (mirror)
			{
				ResizeMirrorDataGLScene(*mirror);
				DrawMirrorData(*mirror);
			}
		}

		//Draw Interface Scene
		ResizeInterfaceGLScene();
		DrawInterfaces();

		// 2. New UI System Rendering (On Top)
		// UIManager::Render() will set up its own orthographic projection.
		if (UIManager::Instance().IsInitialized()) {
			UIManager::Instance().Render();
		}

		resetFlags();
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err << std::endl;
		}
		SwapBuffers(hDC);
		frameEnd = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration frameDuration = frameEnd - frameStart;
		long long sleepTimeMillis = desiredFrameTimeMillis - frameDuration.total_milliseconds();
		if (sleepTimeMillis > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeMillis));
		}
	}
	FontManager::Instance().Cleanup(); // Before deleting context
	UIManager::Instance().Shutdown();

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
	return 0;
}

void pass_command(const char* cmd) {
	InterfaceFrame* frame = frames_def[MAIN_CHAT_INTERFACE];
	if (frame && frame->id == MAIN_CHAT_INTERFACE && frame->render) {
		ChildFrame* child = frame->children[MAIN_CHAT_INPUT];
		if (child && child->type == CHILD_TYPE::INPUT_FIELD && child->focus) {
			InputField& input_box = *(InputField*)child;

			input_box.setInput(cmd);
			input_box.setCursor();
			RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
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
		if (Mirror* mir = (*it).second)
		{
			for (size_t i = 0; i < MIR_FLAG_COUNT; i++) {
				if (mir->update_flags[i]) {
					mir->render_flags[i] = true;
					mir->update_flags[i] = false;
				}
			}
		}
	}
	if (!acf_map.empty()) {
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
	if (!Collision_Map.empty()) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
			Collision* collision = it->second;
			if (collision != nullptr) {
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
	if (!acf_map.empty()) {
		for (auto iter = acf_map.begin(); iter != acf_map.end(); iter++) {
			// iterator->first = key
			Aircraft* aircraft = iter->second;
			if (aircraft != nullptr) {
				for (int i = 0; i < ACF_FLAG_COUNT; i++)
				{
					aircraft->setRenderFlag(i, false);
				}
			}
		}
	}
	if (!Collision_Map.empty()) {
		for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ++it) {
			Collision* collision = it->second;
			if (collision != nullptr) {
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
		if (Mirror* mir = (*it).second)
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

	for (auto it = Collision_Del.begin(); it != Collision_Del.end(); ++it) {
		delete* it;
	}
	Collision_Del.clear();
}

void pull_data(InterfaceFrame& _f, CHILD_TYPE _fc)
{
	if (_f.id == FP_INTERFACE
		&& _fc == CHILD_TYPE::INPUT_FIELD)
	{
		if (opened_fp)
			PullFPData(dynamic_cast<Aircraft*>(opened_fp));
	}
}

void pop_split_line(const InterfaceFrame& frame, InputField* focusField)
{
	CHILD_TYPE type = focusField->type;
	if (focusField->line_ptr) {
		const std::shared_ptr<ChatLine>& line = focusField->line_ptr;
		if (frame.id == FP_INTERFACE) {
			std::shared_ptr<ChatLine> nf = nullptr;
			auto* display_box = dynamic_cast<DisplayBox*>(frame.children[FP_ROUTE_BOX]);
			const auto it = std::find(display_box->chat_lines.begin(), display_box->chat_lines.end(), line);
			auto i = display_box->chat_lines.begin();
			while (i != display_box->chat_lines.end()) {
				if (const std::shared_ptr<ChatLine>& c2 = *i; c2->split == line) {
					printf("split: %s\n", c2->getText().c_str());
					nf = c2;
					break;
				}
				++i;
			}
			if (it != display_box->chat_lines.end()) {
				const int pos = it - display_box->chat_lines.begin();
				focusField->update_line();
				if (const std::shared_ptr<ChatLine> u = display_box->check_unsplit()) {
					display_box->gen_points();
					focusField->history_index = 0;
					focusField->removeFocus();
					if (display_box->handleClick(nullptr, u->get_x(), u->get_y())) {
						if (display_box->getFrame()->children[display_box->index + 1]) {
							dynamic_cast<InputField*>(display_box->getFrame()->children[display_box->index + 1])->setCursorAtEnd();
							RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
						}
					}
				}
				else {
					display_box->consolidate_lines();
					display_box->gen_points();
					focusField->updateInput(display_box->chat_lines[pos]);
				}
			}
		}
		else {
			focusField->handle_box();
			main_chat_input->setFocus();
		}
	}
}

ChildFrame* position_cursor_pop(InterfaceFrame& frame, InputField* focusField) {
	if (focusField->line_ptr) {
		const std::shared_ptr<ChatLine>& line = focusField->line_ptr;
		if (frame.id == FP_INTERFACE) {
			auto* display_box = dynamic_cast<DisplayBox*>(frame.children[FP_ROUTE_BOX]);
			if (!display_box) return nullptr;

			auto it = std::find(display_box->chat_lines.begin(), display_box->chat_lines.end(), line);
			if (it != display_box->chat_lines.end() && it != display_box->chat_lines.begin()) {
				// Not at the first line
				auto prev_it = std::prev(it);
				std::shared_ptr<ChatLine>& prev_line = *prev_it;

				// Store the length of the previous line before merging
				size_t prev_line_length = prev_line->getText().length();

				// Merge texts
				std::string merged_text = prev_line->getText() + line->getText();
				prev_line->setText(merged_text);

				// Remove current line
				it = display_box->chat_lines.erase(it);

				// Update focusField
				focusField->line_ptr = prev_line;
				focusField->setInput(merged_text);
				focusField->setCursor(prev_line_length);

				// Recalculate line wrapping
				display_box->consolidate_lines();
				display_box->gen_points();

				// Refresh display
				RenderFocusChild(CHILD_TYPE::INPUT_FIELD);
				return focusField;
			}
			else {
				// Already at the first line, cannot merge
				return nullptr;
			}
		}
	}
	return nullptr;
}


void wrap_and_clip(InputField* focusField)
{
	if (!focusField || !focusField->line_ptr)
		return;

	std::shared_ptr<ChatLine>& c = focusField->line_ptr;
	if (!c->parent || c->parent->type != CHILD_TYPE::DISPLAY_BOX)
		return;

	DisplayBox* displayBox = static_cast<DisplayBox*>(c->parent);
	if (!displayBox->combined_lines)
		return;

	auto it = std::find(displayBox->chat_lines.begin(), displayBox->chat_lines.end(), c);
	if (it == displayBox->chat_lines.end())
		return;

	int pos = it - displayBox->chat_lines.begin();

	// Update the line and consolidate before generating points
	focusField->update_line();
	displayBox->Wrap_Combined();
	displayBox->gen_points();
	focusField->reverse_update();

}


void open_chat(std::string call_sign)
{
	auto it = find(pm_callsigns.begin(), pm_callsigns.end(), call_sign);
	bool unset = false;
	if (it == pm_callsigns.end())
	{
		it = find(pm_callsigns.begin(), pm_callsigns.end(), "NOT_LOGGED");
		unset = true;
	}

	if (it != pm_callsigns.end())
	{
		InterfaceFrame& frame = *frames_def[it - pm_callsigns.begin()];
		if (unset)
		{
			frame.title = "PRIVATE CHAT: " + call_sign;
			pm_callsigns[it - pm_callsigns.begin()] = call_sign;
		}
		if (!frame.render)
		{
			frame.doOpen(false, true);
		}
	}
	else
	{
		//TODO, make Private Chat User* (Pointer) oriented specific.
		InterfaceFrame* pm_frame = nullptr;
		int id = 0, items_opened = 0, c_id = -1;
		while (items_opened < MAX_PMCHATS) {
			if (is_privateinterface(id))
			{
				pm_frame = frames_def[id];
				if (!pm_frame || !pm_frame->render)
				{
					c_id = id;
					break;
				}
				else {
					++items_opened;
				}
			}
			++id;
		}

		if (items_opened >= MAX_PMCHATS || c_id == -1)
			sendErrorMessage("Too many private chat interfaces");
		else
			LoadPrivateChat(-1, -1, call_sign, true, true, c_id);
	}

}



