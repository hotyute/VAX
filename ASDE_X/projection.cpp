#include "projection.h"

//const double EARTH_RADIUS = 6371300.0;
const double METERS_PER_LAT = 111195.0;
const double MAX_LATITUDE = 90.0;
const double MAX_LONGITUDE = 180.0;

double CENTER_LAT = 0.000000;
double CENTER_LON = 0.000000;

int WIDTH = 1280, HEIGHT = 820;

int FIXED_WIDTH = WIDTH, FIXED_HEIGHT = HEIGHT;

int CLIENT_WIDTH, CLIENT_HEIGHT;
int FIXED_CLIENT_WIDTH, FIXED_CLIENT_HEIGHT;

bool fixed_set = false;

double mZoom = 30.000;
int zoom_phase = 1;
double DEFAULT_ZOOM = 30.0;

double r_aircraft_size = 0.1, default_r_size = 0.003;
double h_aircraft_size = 0.12, default_h_size = 0.0032;
double u_aircraft_size = 0.1, default_u_size = 0.003;

bool DAY = true, CAPS = false, SHIFT_DOWN = false;

char input_cursor = '_';