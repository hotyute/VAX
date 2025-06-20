#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "gui.h"
#include "constants.h"
#include "tempdata.h"

class User;

struct PrivateMessages
{
	std::unordered_map<User*, ChatLine*> message_history;
};

struct Identity {
	std::string callsign;
	std::string login_name;
	std::string password;
	std::string username;
	int id;
	int controller_rating = 0;
	POSITIONS controller_position = POSITIONS::OBSERVER;
	int pilot_rating = 0;
	CLIENT_TYPES type;
};

// Define this struct before UserData
struct CommsLinePersistentData {
	std::string pos;
	std::string freq;
	bool tx_checked = false;
	bool rx_checked = false;
	bool hdst_checked = false;
	bool spkr_checked = false;
	// No isPrimary here; primary is an index stored separately.

	CommsLinePersistentData() : pos("----"), freq("") {} // Default constructor
};

struct UserData {
	int frequency[2];
	int window_positions[4][2];

	// New comms configuration data
	static const int NUM_SAVED_COMMS_LINES = 3;
	CommsLinePersistentData commsConfig[NUM_SAVED_COMMS_LINES];
	int primaryCommsLineIndex = -1; // -1 for no primary


	UserData();
};

class User {
public:
	User(std::string, int, int);
	virtual ~User();
	void setLatitude(double value);
	double getLatitude() const;
	void setLongitude(double value);
	double getLongitude() const;
	virtual void lock() = 0;
	virtual void unlock() = 0;
	std::string getCallsign() { return identity.callsign; }
	int getVisibility() { return visibility; }
	void setVisibility(int vis) { this->visibility = vis; }
	void setCallsign(std::string new_callsign) { identity.callsign = new_callsign; }
	UserData userdata;
	std::vector<std::shared_ptr<ClientScript>> scripts;
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
	void registerScript(User* subject, int index, const ClientScript& proposed);
	void unregisterScript(User* subject, int index);
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

