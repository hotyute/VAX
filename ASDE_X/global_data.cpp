#include "global_data.h"

std::vector<std::pair<std::string, CHAT_TYPE>> g_latestControllerListData;
std::mutex g_controllerListDataMutex;