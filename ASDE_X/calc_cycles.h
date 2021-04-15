#ifndef CALC_CYCLES_H
#define CALC_CYCLES_H

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <thread>
#include <stdio.h>
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "aircraft.h"
#include "collision.h"
#include "tools.h"

DWORD WINAPI CalcThread1(LPVOID);

void update();

void CalculateCollisions();

void CalcDepartures();

#endif

