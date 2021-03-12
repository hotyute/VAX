#include "point2d.h"

std::vector<Point2D*>ALL;

void Point2D::add_holes(double* holes) {
	Point2D::holes.push_back(holes);
}

std::vector<double*> Point2D::get_holes() {
	return Point2D::holes;
}

void Point2D::add_coordinates(double* coordinates1) {
	Point2D::coordinates.push_back(coordinates1);
}

std::vector<double*> Point2D::get_coordinates() {
	return Point2D::coordinates;
}