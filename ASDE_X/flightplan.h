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
#include "user.h"

extern std::unordered_map<std::string, std::vector<std::string>> departures;

extern User* opened_fp;

void Load_Unknown_FlightPlan_Interface(double x, double y, char*, bool);

void Load_Known_No_FlightPlan_Interface(double x, double y, User& user, bool refresh);

void Load_FlightPlan_Interface(double x, double y, User&, bool);

void Load_FlightPlan_Interface(double x, double y, std::string*, bool);

#endif
