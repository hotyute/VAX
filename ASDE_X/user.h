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
	int controller_rating = 0;
	int controller_position = 0;
	int pilot_rating = 0;
	CLIENT_TYPES type;
};
#endif

#ifndef __UserData_user_h
#define __UserData_user_h
struct UserData {
	int frequency[2], window_positions[4][2];
	UserData();
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
	std::string getCallsign() { return identity.callsign; }
	int getVisibility() { return visibility; }
	void setVisibility(int vis) { this->visibility = vis; }
	void setCallsign(std::string new_callsign) { identity.callsign = new_callsign; }
	UserData userdata;
	Identity* getIdentity() {
		return &this->identity;
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
	int visibility = 300;
protected:
	int userIndex;
	Identity identity;
	double latitude;
	double longitude;
	HANDLE aMutex;
};
#endif

#endif

