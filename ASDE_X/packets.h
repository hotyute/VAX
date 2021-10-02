#ifndef PACKETS_H
#define PACKETS_H

#include "user.h"
#include "aircraft.h"

void sendPositionUpdates(User &user);
void sendPingPacket();
void sendUserMessage(int frequency, std::string message);

void sendPrivateMessage(std::string to, std::string message);

void sendFlightPlanRequest(Aircraft& user_for);

void sendDisconnect();

void sendFlightPlan(Aircraft& user);

const int _AIRCRAFT_POS_UPDATE = 1,
_UPDATE_TRANSPONDER = 2,
CONTROLLER_POS_UPDATE = 3,
_UPDATE_MODE = 8,
_PING = 4,
_USER_MESSAGE = 6,
_RECV_TIME_CHANGE = 5,
_SEND_TITLE = 7,
_DISCONNECT_PACKET = 9,
_SEND_FLIGHT_PLAN = 10,
_FLIGHT_PLAN_REQ = 11,
_PRIVATE_MESSAGE = 12;


#endif
