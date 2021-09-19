#include "controller.h"

std::unordered_map<std::string, Controller*>controller_map;

Controller::Controller(std::string callSign, int controllerRating, int pilotRating)
	: User::User(callSign, controllerRating, pilotRating)
{
	this->identity->type = CLIENT_TYPES::CONTROLLER_CLIENT;
}

Controller::~Controller()
{
}

// we implements these functions incase we want to do anything on a type basis
double Controller::getLatitude() {
	return Controller::latitude;
}

void Controller::setLatitude(double value) {
	Controller::latitude = value;
}

double Controller::getLongitude() {
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
