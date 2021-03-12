#include "dxfdrawing.h"

DxfDrawing *aircraftBlip = new DxfDrawing();
DxfDrawing *unknownBlip = new DxfDrawing();

void loadAircraftBlip() {
	aircraftBlip->setMinX(151.750000);
	aircraftBlip->setMaxX(187.000000);
	aircraftBlip->setMinY(135.450000);
	aircraftBlip->setMaxY(170.370370);
	aircraftBlip->addCoordinates(166.363636, 136.000000);
	aircraftBlip->addCoordinates(167.515152, 136.030303);
	aircraftBlip->addCoordinates(168.484848, 137.636364);
	aircraftBlip->addCoordinates(169.303030, 138.787879);
	aircraftBlip->addCoordinates(171.333333, 136.030303);
	aircraftBlip->addCoordinates(179.650000, 135.450000);
	aircraftBlip->addCoordinates(179.550000, 137.550000);
	aircraftBlip->addCoordinates(173.107143, 140.464286);
	aircraftBlip->addCoordinates(173.321429, 147.035714);
	aircraftBlip->addCoordinates(173.392857, 150.107143);
	aircraftBlip->addCoordinates(187.000000, 149.111111);
	aircraftBlip->addCoordinates(186.964286, 151.750000);
	aircraftBlip->addCoordinates(183.750000, 152.892857);
	aircraftBlip->addCoordinates(178.666667, 154.666667);
	aircraftBlip->addCoordinates(173.541667, 156.375000);
	aircraftBlip->addCoordinates(173.370370, 164.777778);
	aircraftBlip->addCoordinates(173.296296, 167.148148);
	aircraftBlip->addCoordinates(172.222222, 169.444444);
	aircraftBlip->addCoordinates(170.888889, 170.370370);
	aircraftBlip->addCoordinates(167.925926, 170.370370);
	aircraftBlip->addCoordinates(166.592593, 169.333333);
	aircraftBlip->addCoordinates(165.629630, 166.925926);
	aircraftBlip->addCoordinates(165.481481, 165.148148);
	aircraftBlip->addCoordinates(165.291667, 156.375000);
	aircraftBlip->addCoordinates(160.250000, 154.821429);
	aircraftBlip->addCoordinates(155.964286, 153.392857);
	aircraftBlip->addCoordinates(151.750000, 151.964286);
	aircraftBlip->addCoordinates(151.750000, 149.464286);
	aircraftBlip->addCoordinates(164.964286, 150.107143);
	aircraftBlip->addCoordinates(165.000000, 146.600000);
	aircraftBlip->addCoordinates(165.200000, 140.466667);
	aircraftBlip->addCoordinates(159.250000, 137.535714);
	aircraftBlip->addCoordinates(159.176471, 135.705882);
	aircraftBlip->addCoordinates(165.909091, 135.969697);
}

void loadUnknownBlip() {
	unknownBlip->setMinX(-9.46875);
	unknownBlip->setMaxX(9.46875);
	unknownBlip->setMinY(-15.307692500000002);
	unknownBlip->setMaxY(15.307692500000002);
	unknownBlip->addCoordinates(-2.03125, -13.415865499999995);
	unknownBlip->addCoordinates(-0.9615379999999902, -14.846153499999986);
	unknownBlip->addCoordinates(-0.1153850000000034, -15.307692500000002);
	unknownBlip->addCoordinates(1.03125, -14.165865499999995);
	unknownBlip->addCoordinates(1.96875, -13.103365499999995);
	unknownBlip->addCoordinates(2.65625, -12.228365499999995);
	unknownBlip->addCoordinates(3.46875, -11.290865499999995);
	unknownBlip->addCoordinates(4.53125, -9.978365499999995);
	unknownBlip->addCoordinates(6.03125, -8.103365499999995);
	unknownBlip->addCoordinates(7.40625, -6.228365499999995);
	unknownBlip->addCoordinates(8.40625, -4.790865499999995);
	unknownBlip->addCoordinates(9.46875, -3.3533654999999953);
	unknownBlip->addCoordinates(7.21875, 1.1466345000000047);
	unknownBlip->addCoordinates(6.46875, 2.8341345000000047);
	unknownBlip->addCoordinates(4.78125, 5.771634500000005);
	unknownBlip->addCoordinates(3.84375, 7.646634500000005);
	unknownBlip->addCoordinates(2.53125, 10.084134500000005);
	unknownBlip->addCoordinates(1.34375, 12.334134500000005);
	unknownBlip->addCoordinates(0.46875, 14.021634500000005);
	unknownBlip->addCoordinates(-0.19230799999999704, 15.307692500000002);
	unknownBlip->addCoordinates(-1.59375, 12.584134500000005);
	unknownBlip->addCoordinates(-2.78125, 10.459134500000005);
	unknownBlip->addCoordinates(-3.84375, 8.396634500000005);
	unknownBlip->addCoordinates(-4.90625, 6.334134500000005);
	unknownBlip->addCoordinates(-6.40625, 2.8341345000000047);
	unknownBlip->addCoordinates(-7.09375, 1.3341345000000047);
	unknownBlip->addCoordinates(-8.03125, -0.5408654999999953);
	unknownBlip->addCoordinates(-8.84375, -2.1658654999999953);
	unknownBlip->addCoordinates(-9.46875, -3.6033654999999953);
	unknownBlip->addCoordinates(-6.59375, -7.415865499999995);
	unknownBlip->addCoordinates(-5.09375, -9.540865499999995);
	unknownBlip->addCoordinates(-3.96875, -10.978365499999995);
	unknownBlip->addCoordinates(-3.03125, -12.290865499999995);
	unknownBlip->addCoordinates(-2.15625, -13.290865499999995);
}

void loadAircraftBlip2() {
	aircraftBlip->setMinX(-17.625);
	aircraftBlip->setMaxX(17.625);
	aircraftBlip->setMinY(-17.460185);
	aircraftBlip->setMaxY(17.460185);
	aircraftBlip->addCoordinates(-3.011364,-16.910185);
	aircraftBlip->addCoordinates(-1.859848,-16.879882);
	aircraftBlip->addCoordinates(-0.890152,-15.273821);
	aircraftBlip->addCoordinates(-0.07197,-14.122306);
	aircraftBlip->addCoordinates(1.958333,-16.879882);
	aircraftBlip->addCoordinates(10.275,-17.460185);
	aircraftBlip->addCoordinates(10.175,-15.360185);
	aircraftBlip->addCoordinates(3.732143,-12.445899);
	aircraftBlip->addCoordinates(3.946429,-5.874471);
	aircraftBlip->addCoordinates(4.017857,-2.803042);
	aircraftBlip->addCoordinates(17.625,-3.799074);
	aircraftBlip->addCoordinates(17.589286,-1.160185);
	aircraftBlip->addCoordinates(14.375,-0.017328);
	aircraftBlip->addCoordinates(9.291667,1.756482);
	aircraftBlip->addCoordinates(4.166667,3.464815);
	aircraftBlip->addCoordinates(3.99537,11.867593);
	aircraftBlip->addCoordinates(3.921296,14.237963);
	aircraftBlip->addCoordinates(2.847222,16.534259);
	aircraftBlip->addCoordinates(1.513889,17.460185);
	aircraftBlip->addCoordinates(-1.449074,17.460185);
	aircraftBlip->addCoordinates(-2.782407,16.423148);
	aircraftBlip->addCoordinates(-3.74537,14.015741);
	aircraftBlip->addCoordinates(-3.893519,12.237963);
	aircraftBlip->addCoordinates(-4.083333,3.464815);
	aircraftBlip->addCoordinates(-9.125,1.911244);
	aircraftBlip->addCoordinates(-13.410714,0.482672);
	aircraftBlip->addCoordinates(-17.625,-0.945899);
	aircraftBlip->addCoordinates(-17.625,-3.445899);
	aircraftBlip->addCoordinates(-4.410714,-2.803042);
	aircraftBlip->addCoordinates(-4.375,-6.310185);
	aircraftBlip->addCoordinates(-4.175,-12.443518);
	aircraftBlip->addCoordinates(-10.125,-15.374471);
	aircraftBlip->addCoordinates(-10.198529,-17.204303);
	aircraftBlip->addCoordinates(-3.465909,-16.940488);
}

double DxfDrawing::getMinX() {
	return DxfDrawing::minX;
}

void DxfDrawing::setMinX(double value) {
	DxfDrawing::minX = value;
}

double DxfDrawing::getMinY() {
	return DxfDrawing::minY;
}

void DxfDrawing::setMinY(double value) {
	DxfDrawing::minY = value;
}

double DxfDrawing::getMaxX() {
	return DxfDrawing::maxX;
}

void DxfDrawing::setMaxX(double value) {
	DxfDrawing::maxX = value;
}

double DxfDrawing::getMaxY() {
	return DxfDrawing::maxY;
}

void DxfDrawing::setMaxY(double value) {
	DxfDrawing::maxY = value;
}

void DxfDrawing::addCoordinates(double x, double y) {
	double* value = new double[3];
	value[0] = x;
	value[1] = y;
	value[2] = 0;
	DxfDrawing::coordinates.push_back(value);
	DxfDrawing::formated_coordinates.push_back(NULL);
}

std::vector<double*> DxfDrawing::getCoordinates() {
	return DxfDrawing::coordinates;
}

std::vector<double*> DxfDrawing::getFCoordinates() {
	return DxfDrawing::formated_coordinates;
}

double* DxfDrawing::formatCoordinates(size_t i, double aircraft_size, double* coords, double latitude, double longitude) {
	double* object = DxfDrawing::formated_coordinates[i];
	double x = coords[0];
	double y = coords[1];
	double valueX = (longitude + (x - DxfDrawing::minX));
	double valueY = (latitude + (y - DxfDrawing::minY));
	double vMaxX = (longitude + (maxX - minX));
	double vMaxY = (latitude + (maxY - minY));
	double offsetX = ((longitude + vMaxX) / 2.0);
	double offsetY = ((latitude + vMaxY) / 2.0);
	double x1 = longitude + ((valueX - offsetX) * aircraft_size);
	double y1 = latitude + ((valueY - offsetY) * aircraft_size);
	if (object == NULL) {
		DxfDrawing::formated_coordinates[i] = new double[2];
		object = DxfDrawing::formated_coordinates[i];
	}
	object[0] = x1;
	object[1] = y1;
	return object;
}