#ifndef MAIN_H
#define MAIN_H

#define IDC_STATIC -1
#define IDR_MYMENU 101
#define IDD_ABOUT 102
#define MyIcon 110
#define ID_FILE_EXIT 40001
#define ID_FILE_CONNECT 40002
#include <windows.h>
#include "gui.h"
#include "constants.h"
#include "flightplan.h"

#define ID_HELP_ABOUT 40003
#define ID_BUTTON 40004
#define ID_TEXT 40005
#define ID_FILE_OPEN 40006

#define IDW_SOUND1 1000
#define IDW_SOUND2 1001
#define IDW_SOUND3 1002
#define IDW_SOUND4 1003
 
//If these are invoked as resources load them
#ifdef RC_INVOKED
IDW_SOUND1 WAVE "Sounds/atcmessage.wav"
#endif

static const int CLIENT_PORT = 6809;
extern bool done;
extern HDC hDC;		// Private GDI Device Context
extern HGLRC hRC;		// Permanent Rendering Context
extern HWND hWnd;		// Holds Our Window Handle
extern PAINTSTRUCT ps;

void connect();
void disconnect();
DWORD WINAPI OpenGLThread(LPVOID lpParameter);
DWORD WINAPI EventThread1(LPVOID lpParameter);

void pass_command(char* cmd);

void pass_chars(char* chars);

void resetFlags();

extern InterfaceFrame *connectFrame;
extern InputField *connect_callsign, *connect_fullname, *connect_username, *connect_password, *textField;
extern Label* callsign_label;
extern CloseButton *connect_closeb;
extern DisplayBox *main_chat;
extern bool connected;

#endif
