#include <vector>
#include <sstream>
#include <iomanip>
#include "filereader.h"
#include "point2d.h"
#include "projection.h"
#include "dxfdrawing.h"
#include "tools.h"


int FileReader::LoadADX(std::string path) {
	std::string line;
	std::ifstream myfile(path);
	std::string header;
	Point2D *point2d = NULL;
	std::vector<Point2D*> list;
	for (size_t i = 0; i < ALL.size(); i++) {
		delete ALL[i];
	}
	ALL.clear();
	if (myfile.is_open()) {
		std::string string1 = "<<";
		std::string string2 = "zoom=";
		std::string string3 = "raircraftsize=";
		std::string string4 = "haircraftsize=";
		std::string string5 = "uaircraftsize=";
		std::string commentStart = ";";
		while (myfile.good()) {
			getline (myfile,line);
			size_t foundComment = line.find(commentStart);
			if (foundComment != std::string::npos) {
				line = line.substr(0, foundComment);
			}
			//line = trim(line);
			size_t found1 = line.find(string1);
			size_t found2 = line.find(string2);
			size_t found3 = line.find(string3);
			size_t found4 = line.find(string4);
			size_t found5 = line.find(string5);
			if (found2 != std::string::npos) {
				int start = (found2+string2.length());
				std::string zoom = line.substr(start);
				if (zoom.length() == 0) {
					//error handling
				} else {
					mZoom = atof(zoom.c_str());
				}
			} else if (found3 != std::string::npos) {
				int start = (found3+string3.length());
				std::string aSize = line.substr(start);
				if (aSize.length() == 0) {
					//error handling
				} else {
					r_aircraft_size = atof(aSize.c_str());
				}
			}
			else if (found4 != std::string::npos) {
				int start = (found4 + string4.length());
				std::string aSize = line.substr(start);
				if (aSize.length() == 0) {
					//error handling
				}
				else {
					h_aircraft_size = atof(aSize.c_str());
				}
			} else if (found5 != std::string::npos) {
				int start = (found5 + string5.length());
				std::string aSize = line.substr(start);
				if (aSize.length() == 0) {
					//error handling
				} else {
					u_aircraft_size = atof(aSize.c_str());
				}
			} else if (found1 != std::string::npos) {
				size_t end = line.find(">>");
				if (end == std::string::npos) {
					//error handling
				} else {
					int start = (found1+string1.length());
					header = line.substr(start, (end - start));
					//std::cout << header << std::endl;
					if (header.length() == 0) {
						//error handling
					} else {
						if (header == "CENTER" || header == "HOLE") {

						} else {
							if (point2d != NULL) {
								ALL.push_back(point2d);
								point2d = NULL;
							}
							point2d = new Point2D();
							if (header == "RUNWAY") {
								point2d->set_type(RUNWAY);
							} else if (header == "PARKING") {
								point2d->set_type(PARKING);
							} else if (header == "TAXIWAY") {
								point2d->set_type(TAXIWAY);
							} else if (header == "APRON") {
								point2d->set_type(APRON);
							}
						}
					}
				}
			} else if (line.length() > 0) {
				std::string slatitude, slongitude;
				std::stringstream stream(line);
				if (header == "HOLE" || header == "RUNWAY" || header == "PARKING" || header == "APRON" 
					|| header == "TAXIWAY" || header == "CENTER") {
						for (int i = 0; i < 2; i++) {
							if (i == 0) {
								getline(stream, slatitude, ' ');
							} else {
								getline(stream, slongitude);
							}
						}
						if (header == "CENTER") {
							CENTER_LAT = atof(slatitude.c_str());
							CENTER_LON = atof(slongitude.c_str());
						} else if (point2d != NULL) {
							if (header == "HOLE") {
								double *coords = new double[3];
								coords[1] = atof(slatitude.c_str());
								coords[0] = atof(slongitude.c_str());
								coords[2] = 0;
								point2d->add_holes(coords);
							} else {
								double *coords = new double[3];
								coords[1] = atof(slatitude.c_str());
								coords[0] = atof(slongitude.c_str());
								//std::cout << std::setprecision(16) << (double)coords[1] << ", " << (double)coords[0] << std::endl;
								coords[2] = 0;
								point2d->add_coordinates(coords);
							}
						}
				}
			}
		}
		if (point2d != NULL) {
			ALL.push_back(point2d);
			point2d = NULL;
		}
		//delete point2d;
		myfile.close();
		return 1;
	} else {
		std::cout << "Unable to open file";
		return 0;
	}
}