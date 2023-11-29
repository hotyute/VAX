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

// we implements these functions incase we want to do anything on a type basis
double Controller::getLatitude() const {
	return Controller::latitude;
}

void Controller::setLatitude(double value) {
	Controller::latitude = value;
}

double Controller::getLongitude() const {
	return Controller::longitude;
}

void Controller::setLongitude(double value) {
	Controller::longitude = value;
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
