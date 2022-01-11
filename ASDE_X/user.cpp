#include "user.h"

#include "usermanager.h"
#include "aircraft.h"
#include "tools.h"

User::User(std::string callSign, int controllerRating, int pilotRating) {
	this->userIndex = -1;
	User::identity.callsign = callSign;
	User::identity.controller_rating = controllerRating;
	User::identity.pilot_rating = pilotRating;
}

User::~User() {
}

void User::handleMovement(double n_lat, double n_lon)
{
	double c_lat = latitude, c_lon = longitude;
	if (c_lat != n_lat || c_lon != n_lon)
	{// if moved
		CLIENT_TYPES type = identity.type;
		if (type == CLIENT_TYPES::PILOT_CLIENT)
		{
			if (SHOW_VECTORS)
			{
				((Aircraft*)this)->setUpdateFlag(ACF_VECTOR, true);
			}
			Aircraft* acf = ((Aircraft*)this);
			if (acf->isCollision())
			{
				for (auto& i : acf->collisions)
				{
					Collision* col = i.second;
					if (!col->getUpdateFlag(COL_COLLISION_LINE))
						col->setUpdateFlag(COL_COLLISION_LINE, true);
				}
			}
		}
	}
	latitude = n_lat;
	longitude = n_lon;
}

UserData::UserData()
{
	std::fill_n(frequency, sizeof(frequency) / sizeof(frequency[0]), 99998);

	for (int i = 0; i < sizeof(window_positions) / sizeof(window_positions[0]); ++i)
	{
		window_positions[i][0] = -1;
		window_positions[i][1] = -1;
	}
}
