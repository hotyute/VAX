#include "projection.h"

//const double EARTH_RADIUS = 6371300.0;
const double METERS_PER_LAT = 111195.0;
const double MAX_LATITUDE = 90.0;
const double MAX_LONGITUDE = 180.0;

double CENTER_LAT = 0.000000;
double CENTER_LON = 0.000000;

int WIDTH = 1280, HEIGHT = 768;

int CLIENT_WIDTH, CLIENT_HEIGHT, CONF_Y;

double mZoom = 30.000;
int range = 2304060;
double rotation = 0, vector_length = 1, ldr_length = 1;
int zoom_phase = 1;
double DEFAULT_ZOOM = 30.0;

double r_aircraft_size = 0.52;
double h_aircraft_size = 0.57;
double u_aircraft_size = 0.52;
double elevation;
std::string icao;

bool DAY = true, DB_BLOCK = true, SHOW_VECTORS = false, CAPS = false, DUMP_COLLISION = false, DUMP_CLOSURE = false, CONTROL = false, SHOW_POINTS = false;

char input_cursor = '_';