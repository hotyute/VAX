#pragma once

constexpr auto max_user_size = 1024;

#include <vector>
#include <unordered_map>

#include "user.h"
#include "basic_stream.h"
#include "controller.h"

extern std::vector<User*> userStorage1;
extern std::unordered_map<std::string, User*> users_map;

void decodePackets(int opCode, BasicStream &stream);

extern Controller* USER;
extern User* ASEL;
