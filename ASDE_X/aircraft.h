#include <Windows.h>
#include <iostream>
#include <vector>
#include <map>

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

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
	int flightRules;
	int cycle;
};
#endif



#ifndef Aircraft_aircraft_h
#define Aircraft_aircraft_h
class Aircraft;
class Aircraft {
private:
	HANDLE aMutex;
	int index;
	bool renderCallsign, renderCollision, renderCollTag;
	bool flags[2];
	bool collision;
	bool heavy;
	int mode;
	std::string callsign, textTag1, textTag2, squawkCode;
	std::string collText, collTag1, collTag2;
	double latitude;
	double longitude;
	double speed;
	double heading;
	std::vector<History*> historyCount;
	FlightPlan *flight_plan;
public:
	Aircraft *collisionAcf;
	Aircraft();
	~Aircraft();
	unsigned int Ccallsign = 0, Ccolltext = 0, collisionDl = 0, collLineDL = 0, collLine2DL = 0;
	int getIndex();
	void setIndex(int);
	bool getRenderCallsign();
	void setRenderCallsign(bool);
	bool getRenderCollTag();
	void setRenderCollTag(bool value);
	bool getRenderCollision();
	void setRenderCollision(bool value);
	FlightPlan* getFlightPlan();
	std::string getCallsign();
	void setCallsign(std::string);
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
	double getLatitude();
	void setLatitude(double);
	double getLongitude();
	void setLongitude(double);
	double getSpeed();
	void setSpeed(double);
	double getHeading();
	void setHeading(double);
	HANDLE getMutex();
	void lock();
	void unlock();
	//void setUser1(User*);
	bool isCollision();
	void setCollision(bool);
	bool isHeavy();
	void setHeavy(bool);
	int getMode();
	void setMode(int);
	bool getCollLine() { return flags[0]; }
	void setCollLine(bool flag) { flags[0] = flag; }
};
#endif


extern std::map<std::string, Aircraft*> AcfMap;

extern Aircraft *getAircraftByIndex(int);

#endif

