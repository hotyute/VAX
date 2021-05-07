#ifndef USERMANAGER_H
#define USERMANAGER_H

#define MAX_USER_SIZE 1024

#include <vector>
#include <unordered_map>

#include "user.h"
#include "Stream.h"
#include "controller.h"

extern std::vector<User*> userStorage1;
extern std::unordered_map<std::string, User*> users_map;

void decodePackets(int opCode, Stream &stream);

extern Controller* USER;

#endif
