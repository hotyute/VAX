#include "user.h"

#include "usermanager.h"
#include "aircraft.h"
#include "tools.h"

User::User(std::string callSign, int controllerRating, int pilotRating) : scripts(MAX_CLIENTSCRIPTS, nullptr) {
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

	// Initialize new comms data
	primaryCommsLineIndex = -1;
	for (int i = 0; i < NUM_SAVED_COMMS_LINES; ++i) {
		// commsConfig[i] is default constructed (pos="----", freq="", bools=false)
	}
}

void User::registerScript(User* subject, int index, const ClientScript& proposed) {
	int script_id = std::any_cast<int>(proposed.objects[0]);

	auto& script = subject->scripts[index];
	if (script_id == 299) {
		ClosureArea clArea = ClosureArea(proposed.assembly);
		clArea.copy(proposed);
		for (int i_11_ = proposed.assembly.length() - 1; i_11_ >= 0; i_11_--)
		{
			if (proposed.assembly.at(i_11_) == 's') {
				std::string s = std::any_cast<std::string>(proposed.objects[i_11_ + 1]);
			}
			else if (proposed.assembly.at(i_11_) == 'l') {
				long long varLon = std::any_cast<long long>(proposed.objects[i_11_ + 1]);
				long long varLat = std::any_cast<long long>(proposed.objects[i_11_--]);
				double lon = *reinterpret_cast<double*>(&varLon);
				double lat = *reinterpret_cast<double*>(&varLat);
				clArea.addPoint(lat, lon);
			}
			else {
				int i = std::any_cast<int>(proposed.objects[i_11_ + 1]);
			}
		}
		script = std::make_shared<ClosureArea>(clArea);
		closureAreas.emplace(index, clArea);
		rendererFlags["redrawClosures"] = true;
	}
}

void User::unregisterScript(User* subject, int index)
{
	auto& script_ptr = subject->scripts[index];
	if (script_ptr != nullptr) {
		auto& script = *script_ptr;
		int script_id = std::any_cast<int>(script.objects[0]);
		if (script_id == 299) {
			auto it = closureAreas.erase(index);			
		}
		script_ptr.reset();
	}
}

double User::getLatitude() const {
	return User::latitude;
}

void User::setLatitude(double value) {
	User::latitude = value;
}

double User::getLongitude() const {
	return User::longitude;
}

void User::setLongitude(double value) {
	User::longitude = value;
}