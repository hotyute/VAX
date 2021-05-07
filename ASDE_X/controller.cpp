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

void Controller::lock() {
	WaitForSingleObject(Controller::aMutex, INFINITE);
}

void Controller::unlock() {
	ReleaseMutex(Controller::aMutex);
}
