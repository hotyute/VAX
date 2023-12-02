#pragma once

#include "user.h"

#ifndef Controller_controller_h
#define Controller_controller_h
class Controller : public User {
public:
	Controller(std::string, int, int);
	virtual ~Controller();
	void setOnBreak(bool value) { on_break = value; }
	bool isBreak() { return on_break; }
	void WindowMove(InterfaceFrame *window, int x, int y);
	void lock();
	void unlock();
private:
	bool on_break = false;
};
#endif

extern std::unordered_map<std::string, Controller*> controller_map;
