#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

class Collision;

#include "constants.h"
#include "user.h"
#include "collision.h"

#ifndef History_aircraft_h
#define History_aircraft_h
class History {
};
#endif

#ifndef FlightPlan_aircraft_h
#define FlightPlan_aircraft_h
class FlightPlan {
public:
	std::string departure, arrival, alternate, squawkCode = "0000";
	std::string acType, scratchPad, cruise, route, remarks;
	int flightRules = 0;
	int cycle = 0;
};
#endif



#ifndef Aircraft_aircraft_h
#define Aircraft_aircraft_h
class Aircraft : public User {
private:
	bool collision = false, heavy = false;
	int mode;
	bool render_flags[ACF_FLAG_COUNT];
	bool update_flags[ACF_FLAG_COUNT];
	std::string callsign, textTag, squawkCode;
	std::string collText, collTag1, collTag2;
	double speed;
	double heading, pitch, roll;
	std::vector<History*> historyCount;
	FlightPlan *flight_plan;
public:
	double wnd_loc[3] { 0, 0, 0 };
	std::vector<double*> wndc;
	std::unordered_map<Aircraft*, Collision*> collisions;
	Aircraft(std::string, int, int);
	void handleModeChange(int new_mode);
	virtual ~Aircraft();
	unsigned int Ccallsign = 0, Ccolltext = 0, collisionDl = 0, vectorDl = 0;
	bool getRenderFlag(int flag);
	void setRenderFlag(int flag, bool val);
	bool getUpdateFlag(int flag) { return update_flags[flag]; }
	void setUpdateFlag(int flag, bool val) { update_flags[flag] = val; }
	FlightPlan* getFlightPlan();
	std::string getTextTag();
	void setTextTag(std::string value);
	std::string getCollText();
	void setCollText(std::string value);
	std::string getCollTag1();
	void setCollTag1(std::string value);
	std::string getCollTag2();
	void setCollTag2(std::string value);
	std::string getSquawkCode();
	void setSquawkCode(std::string);
	double getSpeed();
	void setSpeed(double);
	double getHeading();
	void setHeading(double);
	void setThreeFactor(double hdg, double pit, double roll);
	void lock();
	void unlock();
	//void setUser1(User*);
	bool isCollision();
	void setCollision(bool);
	bool isHeavy();
	void setHeavy(bool);
	int getMode();
	void setMode(int);
};
#endif


extern std::unordered_map<std::string, Aircraft*> acf_map;

extern Aircraft *getAircraftByIndex(int);

