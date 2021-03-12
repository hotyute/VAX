#include "user.h"

User::User(std::string callSign, int type, int controllerRating, int pilotRating) {
	User::aircraft = NULL;
	this->userIndex = -1;
	User::identity = new Identity();
	User::identity->callsign = callSign;
	User::identity->type = type;
	User::identity->controller_rating = controllerRating;
	User::identity->pilot_rating = pilotRating;
}

User::~User() {
	delete User::aircraft;
	delete User::identity;
}