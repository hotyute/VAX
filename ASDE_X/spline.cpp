#include "spline.h"

Spline::Spline() {
}

Spline::Spline(double (&xx)[2], double (&yy)[2]) {
	setValues(xx, yy);
}

void Spline::setValues(double (&xx)[2], double (&yy)[2]) {
	Spline::xx[0] = xx[0];
	Spline::xx[1] = xx[1];
	Spline::yy[0] = yy[0];
	Spline::yy[1] = yy[1];
	Spline::calculateCoefficients();
}

double Spline::getValue(double &x) {
	int index = binarySearch(xx, 2, x);

	if (index > 0) {
		return yy[0];
	}

	index = - (index + 1) - 1;
	//std::cout << index << std::endl;

	if (index < 0) {
		return yy[0];
	}

	return a[index]
                + b[index] * (x - xx[index])
                + c[index] * pow(x - xx[index], 2)
                + d[index] * pow(x - xx[index], 3);
}

void Spline::calculateCoefficients() {
	const int N = 2;
	a[0] = 0.0;
	a[1] = 0.0;
	b[0] = 0.0;
	b[1] = 0.0;
	c[0] = 0.0;
	c[1] = 0.0;
	d[0] = 0.0;
	d[1] = 0.0;
	if (N == 2) {
		Spline::a[0] = yy[0];
		Spline::b[0] = yy[1] - yy[0];
		return;
	}

	double h[N - 1];

	for (int i = 0; i < N - 1; i++) {
		a[i] = yy[i];
		h[i] = xx[i + 1] - xx[i];
		// h[i] is used for division later, avoid a NaN
		if (h[i] == 0.0) {
			h[i] = 0.01;
		}
	}
	a[N - 1] = yy[N - 1];

	/*double A[N - 2][N - 2];
	double y[N - 2];
	for (int i = 0; i < N - 2; i++) {
	y[i] =
	3
	* ((yy[i + 2] - yy[i + 1]) / h[i
	+ 1]
	- (yy[i + 1] - yy[i]) / h[i]);

	A[i][i] = 2 * (h[i] + h[i + 1]);

	if (i > 0) {
	A[i][i - 1] = h[i];
	}

	if (i < N - 3) {
	A[i][i + 1] = h[i + 1];
	}
	}
	solve(A, y);

	for (int i = 0; i < N - 2; i++) {
	c[i + 1] = y[i];
	b[i] = (a[i + 1] - a[i]) / h[i] - (2 * c[i] + c[i + 1]) / 3 * h[i];
	d[i] = (c[i + 1] - c[i]) / (3 * h[i]);
	}
	b[N - 2] =
	(a[N - 1] - a[N - 2]) / h[N
	- 2]
	- (2 * c[N - 2] + c[N - 1]) / 3 * h[N
	- 2];
	d[N - 2] = (c[N - 1] - c[N - 2]) / (3 * h[N - 2]);*/
}