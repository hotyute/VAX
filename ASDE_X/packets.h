#ifndef PACKETS_H
#define PACKETS_H

#include "user.h"
#include "aircraft.h"

void sendPositionUpdates(User &user);
void sendPingPacket();
void sendUserMessage(int frequency, const std::string& to, const std::string& message);

void sendPrivateMessage(const std::string& to, const std::string& message);

void sendFlightPlanRequest(Aircraft& user_for);

void sendDisconnect();

void sendFlightPlan(Aircraft& user);

void sendPrimFreq();

void sendTempData(Aircraft& user, std::string& assembly, const void*& data, ...);

const int _AIRCRAFT_POS_UPDATE = 1,
_UPDATE_TRANSPONDER = 2,
CONTROLLER_POS_UPDATE = 3,
_PING = 4,
_RECV_TIME_CHANGE = 5,
_USER_MESSAGE = 6,
_SEND_TITLE = 7,
_UPDATE_MODE = 8,
_DISCONNECT_PACKET = 9,
_SEND_FLIGHT_PLAN = 10,
_FLIGHT_PLAN_REQ = 11,
_PRIVATE_MESSAGE = 12,
_PRIMARY_FREQ = 13,
_TEMP_DATA = 14;


#endif
