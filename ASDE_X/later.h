#ifndef LATER_H
#define LATER_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include "constants.h"

#ifndef EventAction_h
#define EventAction_h
class EventAction {
private:
	bool running;
	long long ticks;
	boost::posix_time::ptime lastEvent;
public:
	bool paused = false;

	EventAction() {
		running = true;
		lastEvent = boost::posix_time::microsec_clock::local_time();
		ticks = 0L;
	}
	void setRunning(bool value) {
		running = value;
	}
	bool getRunning() {
		return running;
	}
	void setLastEvent(boost::posix_time::ptime value) {
		lastEvent = value;
	}
	boost::posix_time::ptime getLastEvent() {
		return lastEvent;
	}
	void setTicks(long long value) {
		ticks = value;
	}
	long long getTicks() {
		return ticks;
	}

};
#endif

#ifndef Event_h
#define Event_h
class Event {
public:
	EventAction eAction;
	virtual void execute() = 0;
	void toggle_pause() { this->eAction.paused = !this->eAction.paused; }
	virtual void stop() = 0;
};
#endif

#ifndef EventManager_h
#define EventManager_h
class EventManager {
private:
	std::vector<Event*> events;
	int eventCount = 0;

public:
	EventManager() : events(MAX_EVENTS, nullptr) { }
	void addEvent(Event* e) {
		auto it = std::find(events.begin(), events.end(), nullptr);
		if (it != events.end())
			*it = e;
	}
	bool removeEvent(Event* e) {
		return events.erase(std::find(events.begin(), events.end(), e)) != events.end();
	}
	void update();

};
#endif

extern EventManager *event_manager1;

#endif