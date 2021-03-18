#include "point2d.h"

std::vector<PointTess*>ALL;

void PointTess::add_holes(double lat, double lon, double z) {
	Point2 p = Point2(lon, lat);
	LinearSegment* seg = new LinearSegment();
	seg->pt = p;
	PointTess::holes.push_back(seg);
}

std::vector<LinearSegment*> PointTess::get_holes() {
	return PointTess::holes;
}

PointTess::~PointTess()
{
	for (size_t i = 0; i < coordinates.size(); i++) {
		delete coordinates[i];
	}
	for (size_t i = 0; i < holes.size(); i++) {
		delete holes[i];
	}
}

void PointTess::add_coordinates(LinearSegment *seg) 
{
	PointTess::coordinates.push_back(seg);
}

void PointTess::add_coordinates(double lat, double lon, double z)
{
	Point2 pt = Point2(lon, lat);
	LinearSegment *seg = new LinearSegment();
	seg->pt = pt;
	PointTess::coordinates.push_back(seg);
}

void PointTess::add_vector(double lat, double lon, double z, double lat2, double lon2, double z2) 
{
	PointTess::add_coordinates(lat, lon, z);
	PointTess::add_coordinates(lat2, lon2, z2);
}

void PointTess::add_holes(LinearSegment* p)
{
	PointTess::holes.push_back(p);
}

void PointTess::add_coord(LinearSegment* p)
{
	PointTess::coordinates.push_back(p);
}

std::vector<LinearSegment*> PointTess::get_coordinates() 
{
	return PointTess::coordinates;
}

void PointTess::set_coordinates(std::vector<LinearSegment*> new_seg)
{
	PointTess::coordinates = new_seg; 
}

void PointTess::remove_coordinates() 
{
	PointTess::coordinates.clear();
}

LinearSegment* PointTess::get_last_coordinate() 
{
	return PointTess::coordinates.front();
}

double* Point2::as_array()
{
	Point2::p[0] = x_;
	Point2::p[1] = y_;
	Point2::p[2] = 0;
	return Point2::p;
}
