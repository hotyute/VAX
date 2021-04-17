#ifndef __USER_H
#define __USER_H


#include <iostream>
#include <string>
#include <unordered_map>

#include "aircraft.h"
#include "gui.h"

class User;

#ifndef __Pmessages_h
#define __Pmessages_h
struct PrivateMessages
{
	std::unordered_map<User*, ChatLine*> message_history;
};
#endif

#ifndef __Identity_user_h
#define __Identity_user_h
struct Identity {
	std::string callsign;
	std::string login_name;
	std::string password;
	std::string username;
	int id;
	int controller_rating;
	int pilot_rating;
	int type;
};
#endif

#ifndef __User_user_h
#define __User_user_h
class User {
public:
	User(std::string, int, int, int);
	~User();
	void setLatitude(double value) {
		this->latitude = value;
	}
	double getLatitude() {
		return this->latitude;
	}
	void setLongitude(double value) {
		this->longitude = value;
	}
	double getLongitude() {
		return this->longitude;
	}
	Identity* getIdentity() {
		return this->identity;
	}
	void setIdentity(Identity* userIdentity) {
		this->identity = userIdentity;
	}
	int getUserIndex() {
		return userIndex;
	}
	void setUserIndex(int index) {
		userIndex = index;
	}
	long long getUpdateTime() {
		return update_time;
	}
	void setUpdateTime(long long value) {
		update_time = value;
	}
	Aircraft* getAircraft() {
		return aircraft;
	}
	void setAircraft(Aircraft* value) {
		aircraft = value;
	}
	PrivateMessages* getPrivateMsgs() {
		return private_messages;
	}
private:
	int userIndex;
	double latitude;
	double longitude;
	Identity* identity;
	Aircraft* aircraft;
	PrivateMessages* private_messages;
	long long update_time;
};
#endif

#endif

