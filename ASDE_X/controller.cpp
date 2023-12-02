#include "controller.h"

std::unordered_map<std::string, Controller*>controller_map;

Controller::Controller(std::string callSign, int controllerRating, int pilotRating)
	: User::User(callSign, controllerRating, pilotRating)
{
	this->identity.type = CLIENT_TYPES::CONTROLLER_CLIENT;
}

Controller::~Controller()
{
}

void Controller::WindowMove(InterfaceFrame* window, int x, int y)
{
	int wind = -1;

	switch (window->id)
	{
	case FP_INTERFACE:
	{
		wind = _WINPOS_FLIGHTPLAN;
		break;
	}
	case CONTROLLER_INTERFACE:
	{
		wind = _WINPOS_CTRLLIST;
		break;
	}
	case CONNECT_INTERFACE:
	{
		wind = _WINPOS_CONNECT;
		break;
	}
	case COMMS_INTERFACE:
	{
		wind = _WINPOS_COMMS;
		break;
	}
	}

	if (wind != -1)
	{
		userdata.window_positions[wind][0] = x;
		userdata.window_positions[wind][1] = y;
	}
}

void Controller::lock() {
	WaitForSingleObject(Controller::aMutex, INFINITE);
}

void Controller::unlock() {
	ReleaseMutex(Controller::aMutex);
}
