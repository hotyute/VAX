#ifndef EVENTS_H
#define EVENTS_H

#include "later.h"

class PositionUpdates : public Event {
public:
	void execute();
	void toggle_pause();
	void stop();
};

class ConfigUpdates : public Event {
public:
	void execute();
	void toggle_pause();
	void stop();
};

#endif