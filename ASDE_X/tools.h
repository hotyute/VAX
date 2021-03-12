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

extern std::wstring s2ws(const std::string& s);

extern char *s2ca1(const std::string &s);

extern SIZE getBitTextLength(HDC, HFONT, std::string);

extern std::string ltrim(std::string s);

extern std::string rtrim(std::string s);

extern std::string trim(std::string s);

extern std::vector<std::string> split(const std::string&, const std::string&);

extern int binarySearch(double a[], int length, double key);

extern bool pnpoly(int nvert, int *vertx, int *verty, int testx, int testy);

extern long long doubleToRawBits(double x);

extern void wordWrap(std::vector<std::string>& dest, const char* buffer, size_t maxlength, int indent);

double radians(double degrees);

double degrees(double radians);

double dist(double lat1, double lon1, double lat2, double lon2);

void getRunwayBounds(double* p1, double* p2, double w, double** l);

double round_up(double value, int decimal_places);
