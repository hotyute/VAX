#ifndef __TOOLS_H
#define __TOOLS_H

#include <algorithm> 
#include <functional>
#include <cctype>
#include <locale>
#include <windows.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <boost/math/special_functions/round.hpp>

#define _USE_MATH_DEFINES
#include <math.h>


#include "point2d.h"
#include "renderer.h"
#include "constants.h"

std::wstring s2ws(const std::string& s);

std::string ws2s(const std::wstring& wstr);

char *s2ca1(const std::string &s);

SIZE getBitTextLength(HDC, HFONT, std::string);

std::string ltrim(std::string s);

std::string rtrim(std::string s);

std::string trim(std::string s);

void quad_bezier(Point2&, Point2&, Point2&, std::vector<LinearSegment*> &add_to);

void cubic_bezier(Point2&, Point2&, Point2&, Point2&, std::vector<LinearSegment*> &add_to);

extern std::vector<std::string> split(const std::string&, const std::string&, int times);

std::vector<std::string> split(const std::string& s, const std::string& delim);

extern int binarySearch(double a[], int length, double key);

extern bool pnpoly(int nvert, int *vertx, int *verty, int testx, int testy);

extern long long doubleToRawBits(double x);

extern int wordWrap(std::vector<std::string>& dest, const char* buffer, size_t maxlength, int indent);

extern double radians(double degrees);

extern double degrees(double radians);

double dist(double lat1, double lon1, double lat2, double lon2);

void getRunwayBounds(double* p1, double* p2, double w, double** l);

double round_up(double value, int decimal_places);

bool is_closed(LinearSegment& seg);

bool is_curved_node(LinearSegment& node);

bool is_curved(LinearSegment& node);

Point2 recip(const Point2& pt, const Point2& ctrl);

LinearSegment* get_prev(std::vector<LinearSegment*>::iterator it, std::vector<LinearSegment*>& segs);

LinearSegment* get_next(std::vector<LinearSegment*>::iterator it, std::vector<LinearSegment*>& segs);

extern double get_asize(bool heavy, bool standby, double zoom);

double get_asize_no_zoom(bool heavy, bool standby);

double get_default_asize(bool heavy, bool standby);

double get_def_asize_no_zoom(bool heavy, bool standby);

Point2 getLocFromBearing(double latitude, double longitude, double distance, double bearing);

double getBearing(double lat1, double lng1, double lat2, double lng2);

void addAircraftToMirrors(Aircraft*);

void addCollisionToMirrors(Collision* collision);

bool is_privateinterface(int frame_index);

void capitalize(std::string& str);

int random(int start, int end);

double angularCompensation(double heading);

double hdg(double heading);

double zoom_from_range();

double NauticalMilesPerDegreeLon(double lat);

std::string TextToBinaryString(std::string words);

double atodd(std::string in);

std::string frequency_to_string(int frequency);

template<typename T>
T pop_front(std::vector<T>& vec)
{
	assert(!vec.empty());
	auto element = vec.front();
	vec.erase(vec.begin());
	return element;
}

#endif
