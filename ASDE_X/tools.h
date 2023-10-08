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

extern bool pnpoly(int nvert, double *vertx, double *verty, double testx, double testy);

extern long long doubleToRawBits(double x);

extern int wordWrap(std::vector<std::string>& dest, const std::string& buffer, size_t maxlength, int indent);

extern double radians(double degrees);

extern double degrees(double radians);

double plain_dist(double x1, double y1, double x2, double y2);

double dist(double lat1, double lon1, double lat2, double lon2);

void getRunwayBounds(double* p1, double* p2, double w, double** l);

double round_up(double value, int decimal_places);

bool is_closed(LinearSegment& seg);

bool is_curved_node(LinearSegment& node);

bool is_curved(LinearSegment& node);

Point2 recip(const Point2& pt, const Point2& ctrl);

LinearSegment* get_prev(std::vector<LinearSegment*>::iterator it, std::vector<LinearSegment*>& segs);

LinearSegment* get_next(std::vector<LinearSegment*>::iterator it, std::vector<LinearSegment*>& segs);

double get_asize(bool heavy, bool standby);

Point2 getLocFromBearing(double latitude, double longitude, double distance, double bearing);

Point2 GetVector(double latitude, double longitude, double distance, double bearing, double scaleFactor = 1.0, double magVar = 0.0);

double getBearing(double lat1, double lng1, double lat2, double lng2);

void addAircraftToMirrors(Aircraft*);

void addCollisionToMirrors(Collision* collision);

void addCollisionsToMirror(Mirror* mir);

void addCollisionToMirror(Collision* collision, Mirror* mir);

int get_frameid(std::string callsign);

bool is_privateinterface(int frame_index);

void capitalize(std::string& str);

int random(int start, int end);

double angularCompensation(double heading);

double hdg(double heading);

double zoom_from_range();

double NauticalMilesPerDegreeLon(double lat);

std::string TextToBinaryString(std::string words);

double atodd(std::string in);

int string_to_frequency(std::string frequency);

std::string frequency_to_string(int frequency);

Point2* intersect(double $p1_lat, double $p1_lon, double $brng1, double $p2_lat, double $p2_lon, double $brng2);

bool within_boundary(Aircraft& aircraft);

std::string FormatAltitude(std::string altitude);

bool is_digits(const std::string& str);

double GetDecelerationDistance(double initialSpeed, double finalSpeed, double decelRate);

bool intersects(Point2& p1, Point2& p2, Point2& c, double r);

bool whitespace_only(const std::string& str);

double getPanningFactor(double width, double height);

bool doPolygonsIntersect(const std::vector<Point2>& poly1, const std::vector<Point2>& poly2);

bool polygonsRepresentSameOrConvergingTaxiway(const std::vector<Point2>& poly1, const std::vector<Point2>& poly2);

bool isOnSameOrAdjacentPath(Aircraft* obj1, Aircraft* obj2, double time);

bool areColliding(Aircraft* obj1, Aircraft* obj2, double time);

bool on_path_logic(const Point2& point);

bool doSegmentsIntersect(Point2 p1, Point2 q1, Point2 p2, Point2 q2);

bool onSegment(Point2 p, Point2 q, Point2 r);

std::vector<Point2> getPolygonForPoint(const Point2& point);

template<typename T>
T pop_front(std::vector<T>& vec)
{
	assert(!vec.empty());
	auto element = vec.front();
	vec.erase(vec.begin());
	return element;
}

struct PathSegment {
	Point2 start;  // Using Point2 for start and end points of the path.
	Point2 end;
};

#endif
