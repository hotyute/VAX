#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "user.h"

#ifndef Controller_controller_h
#define Controller_controller_h
class Controller;
class Controller : public User {
public:
	Controller(std::string, int, int);
	virtual ~Controller();
	void setLatitude(double value);
	double getLatitude();
	void setLongitude(double value);
	double getLongitude();
	void setOnBreak(bool value) { on_break = value; }
	bool isBreak() { return on_break; }
	void lock();
	void unlock();
private:
	bool on_break;
};
#endif

extern std::unordered_map<std::string, Controller*> controller_map;

#endif
