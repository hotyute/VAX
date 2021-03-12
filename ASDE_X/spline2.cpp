#include "spline2.h"

Spline2D::Spline2D(double (&coords)[2][2]) {
	double x[2];
	double y[2];
	for (int i = 0; i < Spline2D::mLength; i++) {
		x[i] = coords[i][0];
		y[i] = coords[i][1];
	}
	init(x, y);
}

void Spline2D::init(double (&x)[Spline2D::mLength], double (&y)[Spline2D::mLength]) {
	t[0] = 0.0;// start point is always 0.0

	// Calculate the partial proportions of each section between each set
	// of points and the total length of sum of all sections
	for (int i = 1; i < Spline2D::mLength; i++) {
		double lx = x[i] - x[i-1];
		double ly = y[i] - y[i-1];

		// If either diff is zero there is no point performing the square root
		if (0.0 == lx) {
			t[i] = abs(ly);
		} else if (0.0 == ly) {
			t[i] = abs(lx);
		} else {
			t[i] = sqrt(lx*lx+ly*ly);
		}
		length += t[i];
		t[i] += t[i-1];
	}

	for(int i = 1; i< (Spline2D::mLength-1); i++) {
		t[i] = t[i] / length;
	}

	t[(Spline2D::mLength-1)] = 1.0; // end point is always 1.0

	splineX = Spline(t, x);
	splineY = Spline(t, y);
}

void Spline2D::getPoint(double &t, double* dest) {
	dest[0] = splineX.getValue(t);
	dest[1] = splineY.getValue(t);
}