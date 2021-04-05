#ifndef SPLINE_H
#define SPLINE_H

#include <math.h>
#include "tools.h"

class Spline {
private:
	double xx[2];
	double yy[2];
	double a[2];
	double b[2];
	double c[2];
	double d[2];
	void calculateCoefficients();
public:
	Spline();
	Spline(double (&)[2], double (&)[2]);
	double getValue(double&);
	void setValues(double (&)[2], double (&)[2]);
};

#endif