#include "projection.h"

//const double EARTH_RADIUS = 6371300.0;
const double METERS_PER_LAT = 111195.0;
const double MAX_LATITUDE = 90.0;
const double MAX_LONGITUDE = 180.0;

double CENTER_LAT = 0.000000;
double CENTER_LON = 0.000000;

int WIDTH = 1280, HEIGHT = 800;

int FIXED_WIDTH = WIDTH, FIXED_HEIGHT = HEIGHT;

int CLIENT_WIDTH, CLIENT_HEIGHT;
int FIXED_CLIENT_WIDTH, FIXED_CLIENT_HEIGHT;

bool fixed_set = false;

double mZoom = 30.000;

double r_aircraft_size = 0.00022;
double h_aircraft_size = 0.00028;
double u_aircraft_size = 0.00028;

int MOUSE_X = 0, MOUSE_Y = 0;

bool DAY = true, CAPS = false, SHIFT_DOWN = false, MOUSE_MOVE = false;

char input_cursor = '_';