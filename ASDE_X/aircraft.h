#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <Windows.h>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

#include "constants.h"
#include "user.h"

#ifndef History_aircraft_h
#define History_aircraft_h
class History {
};
#endif

#ifndef FlightPlan_aircraft_h
#define FlightPlan_aircraft_h
class FlightPlan {
public:
	std::string departure, arrival, alternate, squawkCode;
	std::string acType, scratchPad, cruise, route, remarks;
	int flightRules = 0;
	int cycle = 0;
};
#endif



#ifndef Aircraft_aircraft_h
#define Aircraft_aircraft_h
class Aircraft;
class Aircraft : public User {
private:
	int index;
	bool collision;
	bool heavy;
	int mode;
	bool render_flags[ACF_FLAG_COUNT];
	bool update_flags[ACF_FLAG_COUNT];
	std::string callsign, textTag1, textTag2, squawkCode;
	std::string collText, collTag1, collTag2;
	double speed;
	double heading;
	std::vector<History*> historyCount;
	FlightPlan *flight_plan;
public:
	Aircraft *collisionAcf;
	Aircraft(std::string, int, int);
	virtual ~Aircraft();
	unsigned int Ccallsign = 0, Ccolltext = 0, collisionDl = 0, vectorDl = 0;
	int getIndex();
	void setIndex(int);
	bool getRenderFlag(int flag);
	void setRenderFlag(int flag, bool val);
	bool getUpdateFlag(int flag) { return update_flags[flag]; }
	void setUpdateFlag(int flag, bool val) { update_flags[flag] = val; }
	FlightPlan* getFlightPlan();
	void setLatitude(double value);
	double getLatitude();
	void setLongitude(double value);
	double getLongitude();
	void setAvailableTag(std::string value);
	std::string getTextTag1();
	void setTextTag1(std::string value);
	std::string getTextTag2();
	void setTextTag2(std::string value);
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

#endif

