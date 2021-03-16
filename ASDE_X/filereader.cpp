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
	PointTess* point2d = NULL;
	for (size_t i = 0; i < ALL.size(); i++) {
		delete ALL[i];
	}
	ALL.clear();
	if (myfile.is_open()) 
	{
		std::string string1 = "<<";
		std::string string2 = "zoom=";
		std::string string3 = "raircraftsize=";
		std::string string4 = "haircraftsize=";
		std::string string5 = "uaircraftsize=";
		std::string commentStart = ";";
		Point2* last_p = NULL;
		while (myfile.good()) {
			getline(myfile, line);
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
			if (found2 != std::string::npos) 
			{
				int start = (found2 + string2.length());
				std::string zoom = line.substr(start);
				if (zoom.length() == 0) {
					//error handling
				}
				else {
					mZoom = atof(zoom.c_str());
				}
			}
			else if (found3 != std::string::npos) 
			{
				int start = (found3 + string3.length());
				std::string aSize = line.substr(start);
				if (aSize.length() == 0) {
					//error handling
				}
				else {
					r_aircraft_size = atof(aSize.c_str());
				}
			}
			else if (found4 != std::string::npos)
			{
				int start = (found4 + string4.length());
				std::string aSize = line.substr(start);
				if (aSize.length() == 0) {
					//error handling
				}
				else {
					h_aircraft_size = atof(aSize.c_str());
				}
			}
			else if (found5 != std::string::npos) 
			{
				int start = (found5 + string5.length());
				std::string aSize = line.substr(start);
				if (aSize.length() == 0) {
					//error handling
				}
				else {
					u_aircraft_size = atof(aSize.c_str());
				}
			}
			else if (found1 != std::string::npos) 
			{
				size_t end = line.find(">>");
				if (end == std::string::npos) {
					//error handling
				}
				else {
					int start = (found1 + string1.length());
					header = line.substr(start, (end - start));
					//std::cout << header << std::endl;
					if (header.length() == 0) {
						//error handling
					}
					else 
					{
						if (header == "CENTER" || header == "HOLE") {

						}
						else 
						{
							if (point2d != NULL) 
							{
								ALL.push_back(point2d);
								point2d = NULL;
							}
							point2d = new PointTess();
							if (header == "RUNWAY") 
							{
								point2d->set_type(RUNWAY);
							}
							else if (header == "RUNWAY LINE") 
							{
								point2d->set_type(RUNWAY);
							}
							else if (header == "PARKING") 
							{
								point2d->set_type(PARKING);
							}
							else if (header == "TAXIWAY") 
							{
								point2d->set_type(TAXIWAY);
							}
							else if (header == "APRON") 
							{
								point2d->set_type(APRON);
							}
						}
					}
				}
			}
			else if (line.length() > 0)
			{
				std::string slatitude, slongitude;
				if (header == "HOLE" || header == "RUNWAY" || "RUNWAY LINE" || header == "PARKING" || header == "APRON"
					|| header == "TAXIWAY" || header == "CENTER") {
					if (header == "RUNWAY LINE") {
						//std::cout << whole_line << std::endl;
						std::vector<std::string> args = split(line, " ");
						double p1[2] = { atof(args[0].c_str()),  atof(args[1].c_str()) };
						double p2[2] = { atof(args[2].c_str()),  atof(args[3].c_str()) };
						double* bounds[5];
						for (int i = 0; i < 5; ++i)
							bounds[i] = new double[2];
						getRunwayBounds(p1, p2, atof(args[4].c_str()) + 2, bounds);
						for (int i = 0; i < 5; i++) {
							point2d->add_coordinates(bounds[i][1], bounds[i][0], 0);
						}
						for (int i = 0; i < 5; ++i)
							delete[] bounds[i];
					}
					else
					{
						std::vector<std::string> args = split(line, " ");
						double lon = atof(args[1].c_str()), lat = atof(args[0].c_str());

						LinearSegment *seg = new LinearSegment();
						seg->pt = Point2(lon, lat);
						if (is_curved(args))
						{
							seg->ctrl = Point2(atof(args[3].c_str()), atof(args[2].c_str()));
							

							//std::string next_line;
							//// Get current position
							//int len = myfile.tellg();
							//// Read line
							//getline(myfile, next_line);
							////Return to position before "Read line".
							//myfile.seekg(len, std::ios_base::beg);

							//cubic_bezier(start_lon, start_lat, c1_lon, c1_lat, c2_lon, c2_lat, end_lon, end_lat, point2d);

						}
						else
						{
							if (header == "CENTER")
							{
								CENTER_LAT = lat;
								CENTER_LON = lon;
							}
							else if (point2d != NULL)
							{
								if (header == "HOLE")
								{
									//point2d->add_holes(cur_pt);
								}
								else
								{
									point2d->add_coordinates(seg);
								}
							}
						}
						//last_p = cur_pt;
					}
				}
			}
		}
		if (point2d != NULL) 
		{
			ALL.push_back(point2d);
			point2d = NULL;
		}
		//delete point2d;
		myfile.close();
		return 1;
	}
	else
	{
		std::cout << "Unable to open file";
		return 0;
	}
}

