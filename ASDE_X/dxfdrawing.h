#include <vector>
#include <iostream>


class DxfDrawing {
private:
	double minX, minY;
	double maxX, maxY;
	std::vector<double*> coordinates;
	std::vector<double*> formated_coordinates;
public:
	double getMinX();
	void setMinX(double);
	double getMinY();
	void setMinY(double);
	double getMaxX();
	void setMaxX(double);
	double getMaxY();
	void setMaxY(double);
	void addCoordinates(double, double);
	std::vector<double*> getCoordinates();
	std::vector<double*> getFCoordinates();
	double* formatCoordinates(size_t, double, double*, double, double);
};

extern DxfDrawing *aircraftBlip;
extern DxfDrawing *unknownBlip;

extern void loadAircraftBlip();
extern void loadUnknownBlip();
extern void loadAircraftBlip2();