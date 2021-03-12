#include "later.h"

class PositionUpdates : public Event {
public:
	void execute();
	void stop();
};

class ConfigUpdates : public Event {
public:
	void execute();
	void stop();
};