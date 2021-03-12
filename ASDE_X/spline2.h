#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include "spline.h"

class Spline2D {
private:
	const static int mLength = 2;
	double t[mLength];
	Spline splineX;
	Spline splineY;
	void init(double (&x)[mLength], double (&y)[mLength]);
	double length;
public:
	Spline2D(double (&s)[mLength][mLength]);
	void getPoint(double&, double*);
};