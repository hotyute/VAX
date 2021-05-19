#ifndef __USER_H
#define __USER_H


#include <iostream>
#include <string>
#include <unordered_map>

#include "gui.h"
#include "constants.h"

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
	int pilot_rating = 0;
	CLIENT_TYPES type;
};
#endif

#ifndef __User_user_h
#define __User_user_h
class User {
public:
	User(std::string, int, int);
	virtual ~User();
	virtual void setLatitude(double value) = 0;
	virtual double getLatitude() = 0;
	virtual void setLongitude(double value) = 0;
	virtual double getLongitude() = 0;
	virtual void lock() = 0;
	virtual void unlock() = 0;
	std::string getCallsign() { return identity->callsign; }
	void setCallsign(std::string new_callsign) { identity->callsign = new_callsign; }
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
	PrivateMessages& getPrivateMsgs() {
		return private_messages;
	}
	void handleMovement(double n_lat, double n_lon);
private:
	PrivateMessages private_messages;
	long long update_time;
protected:
	int userIndex;
	Identity* identity;
	double latitude;
	double longitude;
	HANDLE aMutex;
};
#endif

#endif

