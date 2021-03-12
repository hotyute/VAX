#ifndef USERMANAGER_H
#define USERMANAGER_H

#define MAX_AIRCRAFT_SIZE 1024
#define CONTROLLER_CLIENT 0
#define PILOT_CLIENT 1

#include <vector>
#include <unordered_map>

#include "user.h"
#include "Stream.h"

extern std::vector<User*> userStorage1;
extern std::unordered_map<std::string, User*> users_map;

void decodePackets(int opCode, Stream &stream);

extern User *USER;

#endif
