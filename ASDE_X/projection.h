#ifndef PROJECTION_H
#define PROJECTION_H

#include <iostream>

//extern const double EARTH_RADIUS;
extern const double METERS_PER_LAT;
extern const double MAX_LATITUDE;
extern const double MAX_LONGITUDE;

extern double CENTER_LAT;
extern double CENTER_LON;

extern int WIDTH;
extern int HEIGHT;

extern int CLIENT_WIDTH;
extern int CLIENT_HEIGHT;
extern int CONF_Y;

extern double mZoom;
extern int range;
extern double rotation, vector_length, ldr_length;
extern int zoom_phase;
extern double DEFAULT_ZOOM;

extern double r_aircraft_size, default_r_size;
extern double h_aircraft_size, default_h_size;
extern double u_aircraft_size, default_u_size;
extern double elevation;
extern std::string icao;

extern bool DAY, DB_BLOCK, SHOW_VECTORS, CAPS, SHIFT_DOWN;

extern char input_cursor;

#endif
