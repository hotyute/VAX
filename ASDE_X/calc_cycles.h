#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <thread>
#include <stdio.h>
#include <windows.h>
#define _USE_MATH_DEFINES

#include "aircraft.h"
#include "tools.h"
#include "controller.h"

DWORD WINAPI CalcThread1(LPVOID);

void update();

void CalculateCollisions();

void CalcDepartures();

void CalcControllerList();

void refresh_ctrl_list();

//void check_del_ctrl_list(Controller &controller);

//void refresh_ctrl_list();

void clear_ctrl_list(std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store);

void clear_ctrl_list();

void remove_ctrl_list(std::shared_ptr<ChatLine>& c);

void remove_qlctrl_list(std::shared_ptr<ChatLine>& c);

