#include "user.h"

#include "usermanager.h"
#include "aircraft.h"

User::User(std::string callSign, int controllerRating, int pilotRating) {
	this->userIndex = -1;
	User::identity = new Identity();
	User::identity->callsign = callSign;
	User::identity->controller_rating = controllerRating;
	User::identity->pilot_rating = pilotRating;
}

User::~User() {
	delete User::identity;
}

void User::handleMovement(double n_lat, double n_lon)
{
	double c_lat = latitude, c_lon = longitude;
	if (c_lat != n_lat || c_lon != n_lon)
	{// if moved
		CLIENT_TYPES type = identity->type;
		if (type == CLIENT_TYPES::PILOT_CLIENT) 
		{
			if (SHOW_VECTORS)
			{
				((Aircraft*)this)->setUpdateFlag(ACF_VECTOR, true);
			}
		}
	}
	latitude = n_lat;
	longitude = n_lon;
}