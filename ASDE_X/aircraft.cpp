#include "aircraft.h"

Aircraft::Aircraft() {
	Aircraft::aMutex = CreateMutex(NULL, FALSE, L"Aircraft Mutex");
	Aircraft::mode = 0;
	Aircraft::squawkCode = "0000";
	Aircraft::flight_plan = new FlightPlan();
}

Aircraft::~Aircraft()
{
	delete Aircraft::flight_plan;
}

std::map<std::string, Aircraft*>AcfMap;

int Aircraft::getIndex() {
	return Aircraft::index;
}

void Aircraft::setIndex(int value) {
	Aircraft::index = value;
}

bool Aircraft::getRenderCallsign() {
	return Aircraft::renderCallsign;
}

void Aircraft::setRenderCallsign(bool value) {
	Aircraft::renderCallsign = value;
}

FlightPlan* Aircraft::getFlightPlan()
{
	return Aircraft::flight_plan;
}

std::string Aircraft::getCallsign() {
	return Aircraft::callsign;
}

void Aircraft::setCallsign(std::string value) {
	Aircraft::callsign = value;
}

std::string Aircraft::getSquawkCode() {
	return Aircraft::squawkCode;
}

void Aircraft::setSquawkCode(std::string value) {
	Aircraft::squawkCode = value;
}

double Aircraft::getLatitude() {
	return Aircraft::latitude;
}

void Aircraft::setLatitude(double value) {
	Aircraft::latitude = value;
}

double Aircraft::getLongitude() {
	return Aircraft::longitude;
}

void Aircraft::setLongitude(double value) {
	Aircraft::longitude = value;
}

double Aircraft::getSpeed() {
	return Aircraft::speed;
}

void Aircraft::setSpeed(double value) {
	Aircraft::speed = value;
}

double Aircraft::getHeading() {
	return Aircraft::heading;
}

void Aircraft::setHeading(double value) {
	Aircraft::heading = value;
}

HANDLE Aircraft::getMutex() {
	return Aircraft::aMutex;
}

void Aircraft::lock() {
	WaitForSingleObject(Aircraft::aMutex, INFINITE);
}

void Aircraft::unlock() {
	ReleaseMutex(Aircraft::aMutex);
}

Aircraft *getAircraftByIndex(int index) {
	return NULL;
}

//void Aircraft::setUser1(User *value) {
//Aircraft::acfUser = value;
//}

bool Aircraft::isCollision() {
	return Aircraft::collision;
}

void Aircraft::setCollision(bool value) {
	Aircraft::collision = value;
}

bool Aircraft::isHeavy() {
	return Aircraft::heavy;
}

void Aircraft::setHeavy(bool value) {
	Aircraft::heavy = value;
}

int Aircraft::getMode() {
	return Aircraft::mode;
}

void Aircraft::setMode(int mode) {
	Aircraft::mode = mode;
}