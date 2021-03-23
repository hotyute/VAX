#ifndef TOOLS_W_H
#define TOOLS_W_H
#include <iostream>
#include <algorithm> 
#include <functional>
#include <cctype>
#include <locale>
#include <windows.h>
#include <vector>
#include <sstream>
#include <boost/math/special_functions/round.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include "point2d.h"
#include "renderer.h"

extern std::wstring s2ws(const std::string& s);

extern char *s2ca1(const std::string &s);

extern SIZE getBitTextLength(HDC, HFONT, std::string);

extern std::string ltrim(std::string s);

extern std::string rtrim(std::string s);

extern std::string trim(std::string s);

void quad_bezier(Point2&, Point2&, Point2&, std::vector<LinearSegment*> &add_to);

void cubic_bezier(Point2&, Point2&, Point2&, Point2&, std::vector<LinearSegment*> &add_to);

extern std::vector<std::string> split(const std::string&, const std::string&);

extern int binarySearch(double a[], int length, double key);

extern bool pnpoly(int nvert, int *vertx, int *verty, int testx, int testy);

extern long long doubleToRawBits(double x);

extern void wordWrap(std::vector<std::string>& dest, const char* buffer, size_t maxlength, int indent);

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

#endif
