#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <utility> // For std::pair
#include "gui.h"

extern std::vector<std::pair<std::string, CHAT_TYPE>> g_latestControllerListData;
extern std::mutex g_controllerListDataMutex;