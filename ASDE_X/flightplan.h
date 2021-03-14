#ifndef FLIGHTPLAN_H
#define FLIGHTPLAN_H

#include <iostream>
#include <vector>

#include "projection.h"
#include "gui.h"
#include "constants.h"
#include "renderer.h"
#include "usermanager.h"
#include "main.h"

extern User* opened_fp;

void Load_Unknown_FlightPlan_Interface(char*, bool);

void Load_Known_No_FlightPlan_Interface(User& user, bool refresh);

void Load_FlightPlan_Interface(User&, bool);

void Load_FlightPlan_Interface(std::string*, bool);

#endif
