#ifndef FLIGHTPLAN_H
#define FLIGHTPLAN_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "main.h"
#include "projection.h"
#include "gui.h"
#include "constants.h"
#include "renderer.h"
#include "usermanager.h"
#include "interfaces.h"
#include "aircraft.h"

extern int squawk_range;

extern std::unordered_map<std::string, std::vector<std::string>> departures;
extern std::unordered_map<std::string, ChatLine*> obs_list, ql_obs_list, del_list, ql_del_list, gnd_list, ql_gnd_list,
twr_list, ql_twr_list, dep_list, ql_dep_list, app_list, ql_app_list;

extern User* opened_fp;

void Load_Unknown_FlightPlan_Interface(double x, double y, char*, bool);

void Load_Known_No_FlightPlan_Interface(double x, double y, Aircraft& user, bool refresh);

void Load_FlightPlan_Interface(double x, double y, Aircraft&, bool);

void Load_FlightPlan_Interface(double x, double y, std::string*, bool);

void PullFPData(Aircraft* to);

#endif
