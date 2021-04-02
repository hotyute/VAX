#include "aircraft.h"

Aircraft::Aircraft() {
	Aircraft::aMutex = CreateMutex(NULL, FALSE, L"Aircraft Mutex");
	Aircraft::mode = 0;
	Aircraft::squawkCode = "0000";
	Aircraft::flight_plan = new FlightPlan();
	collision = false;
	collisionAcf = nullptr;
	for (size_t i = 0; i < ACF_FLAG_COUNT; i++) {
		update_flags[i] = false;
		render_flags[i] = false;
	}
	update_flags[ACF_COLLISION_TAG] = true;
	Aircraft::textTag1 = "";
	Aircraft::textTag2 = "";
	Aircraft::collText = "#CALLSIGN";
	Aircraft::collTag1 = "No Alert";
	Aircraft::collTag2 = "DISPLAY#0000";
}

Aircraft::~Aircraft()
{
	std::cout << "hello" << std::endl;
	delete Aircraft::flight_plan;
}

std::map<std::string, Aircraft*>AcfMap;

int Aircraft::getIndex() {
	return Aircraft::index;
}

void Aircraft::setIndex(int value) {
	Aircraft::index = value;
}

bool Aircraft::getRenderFlag(int flag)
{
	return Aircraft::render_flags[flag];
}

void Aircraft::setRenderFlag(int flag, bool val)
{
	Aircraft::render_flags[flag] = val;
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

std::string Aircraft::getTextTag1() {
	return Aircraft::textTag1;
}

void Aircraft::setTextTag1(std::string value) {
	Aircraft::textTag1 = value;
}

std::string Aircraft::getTextTag2() {
	return Aircraft::textTag2;
}

void Aircraft::setTextTag2(std::string value) {
	Aircraft::textTag2 = value;
}

std::string Aircraft::getCollText() {
	return Aircraft::collText;
}

void Aircraft::setCollText(std::string value) {
	Aircraft::collText = value;
}

std::string Aircraft::getCollTag1() {
	return Aircraft::collTag1;
}

void Aircraft::setCollTag1(std::string value) {
	Aircraft::collTag1 = value;
}

std::string Aircraft::getCollTag2() {
	return Aircraft::collTag2;
}

void Aircraft::setCollTag2(std::string value) {
	Aircraft::collTag2 = value;
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