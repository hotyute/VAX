#ifndef TOOLS_H
#define TOOLS_H

#include "tools.h"

#define N_SEG 20 // num points

void quad_bezier(Point2& p1, Point2& p2, Point2& p3, std::vector<LinearSegment*>& add_to) {
	unsigned int i;
	double pts[N_SEG + 1][2];
	for (i = 0; i <= N_SEG; ++i)
	{
		double t = (double)i / (double)N_SEG;
		double a = pow((1.0 - t), 2.0);
		double b = 2.0 * t * (1.0 - t);
		double c = pow(t, 2.0);
		double x = a * p1.x_ + b * p2.x_ + c * p3.x_;
		double y = a * p1.y_ + b * p2.y_ + c * p3.y_;
		pts[i][0] = x;
		pts[i][1] = y;
	}

	/* draw segments */
	for (i = 1; i < N_SEG; ++i)
	{
		int j = i + 1;
		LinearSegment* f = new LinearSegment();// , * g = new LinearSegment();
		f->pt = Point2(pts[i][0], pts[i][1]);// , g->pt = Point2(pts[j][0], pts[j][1]);
		add_to.push_back(f);// , add_to.push_back(g);
	}
}

void cubic_bezier(Point2& p1, Point2& p2, Point2& p3, Point2& p4, std::vector<LinearSegment*>& add_to)
{
	unsigned int i;
	double pts[N_SEG + 1][2];
	for (i = 0; i <= N_SEG; ++i)
	{
		double t = (double)i / (double)N_SEG;

		double a = pow((1.0 - t), 3.0);
		double b = 3.0 * t * pow((1.0 - t), 2.0);
		double c = 3.0 * pow(t, 2.0) * (1.0 - t);
		double d = pow(t, 3.0);

		double x = a * p1.x_ + b * p2.x_ + c * p3.x_ + d * p4.x_;
		double y = a * p1.y_ + b * p2.y_ + c * p3.y_ + d * p4.y_;
		pts[i][0] = x;
		pts[i][1] = y;
	}

	/* draw segments */
	for (i = 1; i < N_SEG; ++i)
	{
		//int j = i + 1;
		LinearSegment* f = new LinearSegment();// , * g = new LinearSegment();
		f->pt = Point2(pts[i][0], pts[i][1]);// , g->pt = Point2(pts[j][0], pts[j][1]);
		add_to.push_back(f);// , add_to.push_back(g);
	}
}

std::vector<std::string>& split(const std::string& str, const std::string& delimiters, std::vector<std::string>& elems) {
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		elems.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
	return elems;
}


std::vector<std::string> split(const std::string& s, const std::string& delim) {
	std::vector<std::string> elems;
	return split(s, delim, elems);
}

std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

SIZE getBitTextLength(HDC hDC, HFONT font, std::string text) {
	HFONT oldfont = (HFONT)SelectObject(hDC, font);
	SIZE extent;
	std::wstring wStr = s2ws(text);
	GetTextExtentPoint32(hDC, wStr.c_str(), text.length(), &extent);
	SelectObject(hDC, oldfont);
	return extent;
}

// trim from start
std::string ltrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c); }));
	return s;
}

// trim from end
std::string rtrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
	return s;
}

// trim from both ends
std::string trim(std::string s) {
	return ltrim(rtrim(s));
}

int binarySearch1(double sortedArray[], int first, int last, double key) {
	// function:
	//   Searches sortedArray[first]..sortedArray[last] for key.  
	// returns: index of the matching element if it finds key, 
	//         otherwise  -(index where it could be inserted)-1.
	// parameters:
	//   sortedArray in  array of sorted (ascending) values.
	//   first, last in  lower and upper subscript bounds
	//   key         in  value to search for.
	// returns:
	//   index of key, or -insertion_position -1 if key is not 
	//                 in the array. This value can easily be
	//                 transformed into the position to insert it.

	while (first <= last) {
		int mid = (first + last) / 2;  // compute mid point.
		if (key > sortedArray[mid])
			first = mid + 1;  // repeat search in top half.
		else if (key < sortedArray[mid])
			last = mid - 1; // repeat search in bottom half.
		else
			return mid;     // found it. return position /////
	}
	return -(first + 1);   // failed to find key
}

int binarySearch(double sortedArray[], int length, double key) {
	return binarySearch1(sortedArray, 0, length, key);
}

bool pnpoly(int nvert, int* vertx, int* verty, int testx, int testy) {
	bool c = false;
	int i, j;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i])) {
			c = !c;
		}
	}
	return c;
}

long long doubleToRawBits(double x) {
	long long bits;
	memcpy(&bits, &x, sizeof bits);
	return bits;
}

char* s2ca1(const std::string& s) {
	char* res = new char[s.size() + 1];
	strncpy_s(res, s.size() + 1, s.c_str(), s.size() + 1);
	return res;
}

void wordWrap(std::vector<std::string>& dest, const char* buffer, unsigned int maxlength, int indent) {

	bool has_space = false;

	for (int i = 0; i < maxlength; i++) {
		if (isspace(buffer[i])) {
			has_space = true;
			break;
		}
	}

	if (!has_space) {
		return;
	}

	//std::cout << p << std::endl;

	size_t count, buflen;
	const char* ptr, * endptr;
	count = 0;
	buflen = strlen(buffer);
	do {
		ptr = buffer + count;

		/* don't set endptr beyond the end of the buffer */

		if (ptr - buffer + maxlength <= buflen)
			endptr = ptr + maxlength;
		else
			endptr = buffer + buflen;
		/* back up EOL to a null terminator or space */

		while (*(endptr) && !isspace(*(endptr)))
			endptr--;

		const int size = (endptr - ptr);
		char* out = (char*)malloc(size);

		strncpy(out, ptr, size);
		out[size] = '\0';
		rtrim(out);
		dest.push_back(out);

		count += size;
		//TODO Fix bug where word is longer than max length (word as in no spaces)
	} while (*endptr);
}

double radians(double degrees) {
	return (degrees * M_PI) / 180;
}

double degrees(double radians) {
	return (radians * 180) / M_PI;
}

double dist(double lat1, double lon1, double lat2, double lon2) {
	double dist, dlon = lon2 - lon1;
	lat1 *= M_PI / 180.0;
	lat2 *= M_PI / 180.0;
	dlon *= M_PI / 180.0;
	dist = (sin(lat1) * sin(lat2)) + (cos(lat1) * cos(lat2) * cos(dlon));
	if (dist > 1.0) dist = 1.0;
	dist = acos(dist) * 60 * 180 / M_PI;
	return dist;
}

void getRunwayBounds(double* p1, double* p2, double w, double** l) {
	int degree_dist_at_equator = 111120;
	int lat_degree_dist_average = 111000;
	double degree_dist_at_lat = cos(radians(p1[0])) * degree_dist_at_equator;
	double dx, dy;
	if (round_up(p1[1], 6) == round_up(p2[1], 6)) { //runway exactly east - west direction
		dx = 0;   //difference for longitute in meters to reach corner from center end
		dy = w / 2; //difference for latitude in meters to reach corner from center end
	}
	else if (round_up(p1[0], 6) == round_up(p2[0], 6)) { //runway is exactly north - south direction
		dx = w / 2;
		dy = 0;
	}
	else {
		double m = -1 / ((p2[0] - p1[0]) / (p2[1] - p1[1])); //gradient of the perpendicular runway line
		dx = sqrt(std::pow((w / 2.0), 2.0) / (1.0 + std::pow(m, 2.0)));
		dy = dx * m;
	}
	dx /= degree_dist_at_lat; //convert distance into meters in longitute coordinate difference at geographical latitude
	dy /= lat_degree_dist_average; //convert meters in latitude coordinate difference
	if ((p1[1] <= p2[1] && dy >= 0) || (p1[1] > p2[1] && dy < 0)) { //make sure to always add in clockwise order
		l[0][0] = round_up(p1[1] - dx, 8), l[0][1] = round_up(p1[0] - dy, 8); //buttom corner1
		l[1][0] = round_up(p1[1] + dx, 8), l[1][1] = round_up(p1[0] + dy, 8); //buttom corner2
		l[2][0] = round_up(p2[1] + dx, 8), l[2][1] = round_up(p2[0] + dy, 8); //top corner1
		l[3][0] = round_up(p2[1] - dx, 8), l[3][1] = round_up(p2[0] - dy, 8); //top corner2
	}
	else { //insert all those vertices in different order please to assure clockwise orientation
		l[0][0] = round_up(p1[1] + dx, 8), l[0][1] = round_up(p1[0] + dy, 8);
		l[1][0] = round_up(p1[1] - dx, 8), l[1][1] = round_up(p1[0] - dy, 8);
		l[2][0] = round_up(p2[1] - dx, 8), l[2][1] = round_up(p2[0] - dy, 8);
		l[3][0] = round_up(p2[1] + dx, 8), l[3][1] = round_up(p2[0] + dy, 8);
	}
	l[4][0] = l[0][0], l[4][1] = l[0][1]; //add the very first corner to form closed loop
}

double round_up(double value, int decimal_places) {
	const double multiplier = std::pow(10.0, decimal_places);
	return std::ceil(value * multiplier) / multiplier;
}

bool is_closed(LinearSegment& seg) {
	return seg.loop_type == LOOP_TYPE::CLOSE;
}

bool is_curved_node(LinearSegment& seg) {
	return seg.type == LINE_TYPE::NODE_CTRL;
}

bool is_curved(LinearSegment& seg) {
	return seg.ctrl_hdl.has_lo || seg.ctrl_hdl.has_hi;
}

Point2 recip(const Point2& pt, const Point2& ctrl) {
	return pt + Vector2(ctrl, pt);
}

LinearSegment* get_prev(std::vector<LinearSegment*>::iterator it, std::vector<LinearSegment*>& segs) {
	LinearSegment* prev;
	if (it == segs.begin())
		prev = *(segs.end() - 1);
	else
		prev = *(it - 1);
	return prev;
}

LinearSegment* get_next(std::vector<LinearSegment*>::iterator it, std::vector<LinearSegment*>& segs) {
	LinearSegment* next;
	if ((it + 1) == segs.end())
		next = *segs.begin();
	else
		next = *(it + 1);
	return next;
}

double get_asize(bool heavy, bool standby, double zo)
{
	double size;
	if (standby) {
		double u_size = (zo * u_aircraft_size);
		size = u_size;
	}
	else
	{
		if (heavy) {
			double h_size = (zo * h_aircraft_size);
			size = h_size;
		}
		else
		{
			double r_size = (zo * r_aircraft_size);
			size = r_size;
		}
	}
	return size;
}

double get_asize_no_zoom(bool heavy, bool standby)
{
	double size;
	if (standby) {
		double u_size = u_aircraft_size;
		size = u_size;
	}
	else
	{
		if (heavy) {
			double h_size = h_aircraft_size;
			size = h_size;
		}
		else
		{
			double r_size = r_aircraft_size;
			size = r_size;
		}
	}
	return size;
}

double get_default_asize(bool heavy, bool standby)
{
	double size;
	if (standby) {
		double u_size = (DEFAULT_ZOOM * default_u_size);
		size = u_size;
	}
	else
	{
		if (heavy) {
			double h_size = (DEFAULT_ZOOM * default_h_size);
			size = h_size;
		}
		else
		{
			double r_size = (DEFAULT_ZOOM * default_r_size);
			size = r_size;
		}
	}
	return size;
}

double get_def_asize_no_zoom(bool heavy, bool standby)
{
	double size;
	if (standby) {
		double u_size = default_u_size;
		size = u_size;
	}
	else
	{
		if (heavy) {
			double h_size = default_h_size;
			size = h_size;
		}
		else
		{
			double r_size = default_r_size;
			size = r_size;
		}
	}
	return size;
}

template <typename T>
void remove(std::vector<T>& vec, void* pos)
{

}

double get_delta(double from, double to, double move_by) {
	double going = to - from;
	if (going < 0) {
		from -= move_by;
	}
	else if (going > 0)
	{
		from += move_by;
	}
	return from;
}


Point2 getLocFromBearing(double latitude, double longitude, double distance, double bearing) {
	double R = 6378.14;

	// Degree to Radian
	double latitude1 = radians(latitude);
	double longitude1 = radians(longitude);
	double brng = radians(bearing);

	double latitude2 = asin(sin(latitude1) * cos(distance / R) + cos(latitude1) * sin(distance / R) * cos(brng));
	double longitude2 = longitude1 + atan2(sin(brng) * sin(distance / R) * cos(latitude1), cos(distance / R) - sin(latitude1) * sin(latitude2));

	// back to degrees
	latitude2 = degrees(latitude2);
	longitude2 = degrees(longitude2);

	// 8 decimal for Leafletand other system compatibility
	double lat2 = round_up(latitude2, 8);
	double long2 = round_up(longitude2, 8);

	return Point2(long2, lat2);
}

double getBearing(double lat1, double lng1, double lat2, double lng2) {
	lat1 = radians(lat1);
	lng1 = radians(lng1);
	lat2 = radians(lat2);
	lng2 = radians(lng2);

	double dLng = lng2 - lng1;
	double dPhi = log(tan(lat2 / 2.0 + M_PI / 4.0) / tan(lat1 / 2.0 + M_PI / 4.0));

	if (abs(dLng) > M_PI) {
		if (dLng > 0.0)
			dLng = -(2.0 * M_PI - dLng);
		else
			dLng = (2.0 * M_PI + dLng);
	}

	return fmod((degrees(atan2(dLng, dPhi)) + 360.0), 360.0);
}

void addAircraftToMirrors(Aircraft* acf)
{
	for (auto it = mirrors_storage.begin(); it != mirrors_storage.end(); ++it)
	{
		Mirror* mir = (*it).second;

		if (mir) {
			mir->g_flags.emplace(acf, std::vector<unsigned int>(3)); // initialized to 0 by default
		}
	}
}

bool is_privateinterface(int frame_index) {
	return frame_index == PRIVATE_MESSAGE_INTERFACE || frame_index == PRIVATE_MESSAGE_INTERFACE2 || frame_index == PRIVATE_MESSAGE_INTERFACE3 ||
		frame_index == PRIVATE_MESSAGE_INTERFACE4 || frame_index == PRIVATE_MESSAGE_INTERFACE5 || frame_index == PRIVATE_MESSAGE_INTERFACE6
		|| frame_index == PRIVATE_MESSAGE_INTERFACE7 || frame_index == PRIVATE_MESSAGE_INTERFACE8 || frame_index == PRIVATE_MESSAGE_INTERFACE9
		|| frame_index == PRIVATE_MESSAGE_INTERFACE10;
}

#endif
