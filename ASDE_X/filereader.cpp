#include <vector>
#include <sstream>
#include <iomanip>

#include "filereader.h"

#include "point2d.h"
#include "projection.h"
#include "dxfdrawing.h"
#include "tools.h"
#include "topbutton.h"
#include "calc_cycles.h"

std::string LAST_ADX_PATH, LAST_POF_PATH, LAST_ALIAS_PATH, LAST_CPF_PATH;


void open_adx(std::string path)
{
	if (FileReader::LoadADX(path)) {
		LAST_ADX_PATH = path;
		preFileRender();
		resize = true;
		renderSector = true;
		renderSectorColours = true;
		renderButtons = true;
		updateFlags[GBL_CALLSIGN] = true;
		updateFlags[GBL_COLLISION_TAG] = true;
		renderLegend = true;
		renderInterfaces = true;
		renderDrawings = true;
		renderConf = true;
		renderDate = true;
		renderCoordinates = true;
		renderDepartures = true;
		updateFlags[GBL_COLLISION_LINE] = true;
		convert_closures = true;
		renderClosures = true;
		zoom_phase = 2;
		rangeb->refreshOption2();
		refresh_ctrl_list();
	}
}


int FileReader::LoadADX(std::string path) {
	std::string line;
	std::ifstream myfile(path);
	std::string header;
	PointTess* point2d = NULL;
	auto p_it = ALL.begin();
	while (p_it != ALL.end()) {
		PointTess* p = *p_it;
		p_it = ALL.erase(p_it);
		delete p;
	}
	auto cl_it = closures.begin();
	while (cl_it != closures.end())
	{
		double* c = *cl_it;
		cl_it = closures.erase(cl_it);
		delete[] c;
	}
	auto m_it = mirrors_storage.begin();
	while (m_it != mirrors_storage.end()) {
		Mirror* m = (*m_it).second;
		m_it = mirrors_storage.erase(m_it);
		delete m;
	}
	mirrors.clear();
	if (myfile.is_open())
	{
		std::string string1 = "<<";
		std::string string2 = "zoom=";
		std::string string3 = "raircraftsize=";
		std::string string4 = "haircraftsize=";
		std::string string5 = "uaircraftsize=";
		std::string string6 = "ICAO=";
		std::string string7 = "elevation=";
		std::string string8 = "range=";
		std::string string9 = "rotation=";
		std::string string10 = "leaderlength=";
		std::string commentStart = ";";
		int line_number = 1;
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
			size_t found6 = line.find(string6);
			size_t found7 = line.find(string7);
			size_t found8 = line.find(string8);
			size_t found9 = line.find(string9);
			size_t found10 = line.find(string10);
			try
			{
				if (found2 != std::string::npos)
				{
					int start = (found2 + string2.length());
					std::string zoom = line.substr(start);
					if (zoom.length() == 0) {
						//error handling
					}
					else {
						range = atodd(zoom.c_str());
						mZoom = zoom_from_range();
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
						r_aircraft_size = atodd(aSize.c_str());
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
						h_aircraft_size = atodd(aSize.c_str());
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
						u_aircraft_size = atodd(aSize.c_str());
					}
				}
				else if (found6 != std::string::npos)
				{
					int start = (found6 + string6.length());
					std::string apt_icao = line.substr(start);
					if (apt_icao.length() == 0) {
						//error handling
					}
					else {
						icao = apt_icao.c_str();
					}
				}
				else if (found7 != std::string::npos)
				{
					int start = (found7 + string7.length());
					std::string elevation = line.substr(start);
					if (elevation.length() == 0) {
						//error handling
					}
					else {
						elevation = atodd(elevation.c_str());
					}
				}
				else if (found8 != std::string::npos)
				{
					int start = (found8 + string8.length());
					std::string ra = line.substr(start);
					if (ra.length() == 0) {
						//error handling
					}
					else {
						rangeb->set_range((int)atodd(ra.c_str()), 3, 600, range);
					}
				}
				else if (found9 != std::string::npos)
				{
					int start = (found9 + string9.length());
					std::string ro = line.substr(start);
					if (ro.length() == 0) {
						//error handling
					}
					else {
						rotateb->set_rotation(hdg((int)atodd(ro.c_str())), 0, 359);
					}
				}
				else if (found10 != std::string::npos)
				{
					int start = (found10 + string10.length());
					std::string ll = line.substr(start);
					if (ll.length() != 0)
					{
						ldr_length = atodd(ll.c_str());
						leaderlb->setOption2(std::to_string(leaderlb->value = (int)ldr_length));
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
							if (header == "CENTER" || header == "HOLE" || header == "MIRROR" || header == "CLOSURES")
							{

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
						|| header == "TAXIWAY" || header == "CENTER" || header == "MIRROR" || header == "CLOSURES") {
						if (header == "RUNWAY LINE") {
							//std::cout << whole_line << std::endl;
							std::vector<std::string> args = split(line, " ");
							std::string rw1 = args[0], rw2 = args[3];
							double p1[2] = { atodd(args[1].c_str()),  atodd(args[2].c_str()) };
							double p2[2] = { atodd(args[4].c_str()),  atodd(args[5].c_str()) };
							double shoulder = atodd(args[7].c_str());
							double heading = atodd(args[8].c_str());
							double** bounds = new double* [5];
							for (int i = 0; i < 5; ++i)
								bounds[i] = new double[2];
							getRunwayBounds(p1, p2, atodd(args[6].c_str()) + shoulder, bounds);
							if (runway_polygons.find(rw1) != runway_polygons.end()) {
								// If rw1 already exists in the map, append the polygons to the existing vector
								for (int i = 0; i < 5; i++) {
									runway_polygons[rw1].push_back(point2d->add_coordinates(bounds[i][1], bounds[i][0], 0));
								}
							}
							else {
								// If rw1 does not exist in the map, create a new vector and add polygons to it
								std::vector<Point2> new_vector;
								for (int i = 0; i < 5; i++) {
									new_vector.push_back(point2d->add_coordinates(bounds[i][1], bounds[i][0], 0));
								}
								runway_polygons[rw1] = new_vector;
							}

							//for (int i = 0; i < 5; ++i)
							//{
								//delete[] bounds[i];
							//}
						}
						else if (header == "CENTER")
						{
							std::vector<std::string> args = split(line, " ");
							double lon = atodd(args[1].c_str()), lat = atodd(args[0].c_str());
							CENTER_LAT = lat;
							CENTER_LON = lon;
						}
						else if (header == "MIRROR")
						{
							std::vector<std::string> args = split(line, " ");
							double lon = atodd(args[2].c_str()), lat = atodd(args[1].c_str());
							std::string id = args[0];
							Mirror* mir = new Mirror();
							mir->setX(atodd(args[3].c_str())), mir->setY(atodd(args[4].c_str()));
							mir->setWidth(atodd(args[5].c_str())), mir->setHeight(atodd(args[6].c_str()));
							mir->setZoom(atodd(args[7].c_str()));
							mir->setLat(lat);
							mir->setLon(lon);
							mirrors_storage.emplace(id, mir);
							mir->id_ = id;
						}
						else if (header == "CLOSURES")
						{
							std::vector<std::string> args = split(line, " ");
							double lon = atodd(args[1].c_str()), lat = atodd(args[0].c_str());

							double* c = new double[] { lat, lon };

							closures.push_back(c);
						}
						else
						{
							if (point2d != NULL)
							{
								std::vector<std::string> args = split(line, " ");
								char loop_type = args[0].c_str()[0];
								double lon = atodd(args[2].c_str()), lat = atodd(args[1].c_str());

								LinearSegment* seg = new LinearSegment();

								switch (loop_type)
								{
								case 'L':
									seg->loop_type = LOOP_TYPE::LOOP;
									break;
								case 'C':
									seg->loop_type = LOOP_TYPE::CLOSE;
									break;
								case 'E':
									seg->loop_type = LOOP_TYPE::END;
									break;
								default:
									seg->loop_type = LOOP_TYPE::LOOP;
									break;
								}

								seg->pt = Point2(lon, lat);
								if (args.size() > 3)
								{
									seg->ctrl = Point2(atodd(args[4].c_str()), atodd(args[3].c_str()));
									seg->type = LINE_TYPE::NODE_CTRL;
									point2d->add_coordinates(seg);


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
									if (header == "HOLE")
									{
										point2d->add_holes(seg);
									}
									else
									{
										seg->type = LINE_TYPE::NODE;
										point2d->add_coordinates(seg);
									}
								}
							}
						}
					}
				}
			}
			catch (...)
			{
				std::stringstream box_message;
				box_message << "Error in adx file at line: " << line_number;
				MessageBoxA(hWnd, box_message.str().c_str(), "Notice",
					MB_OK | MB_ICONINFORMATION);
			}
			++line_number;
		}
		if (point2d != NULL)
		{
			ALL.push_back(point2d);
			point2d = NULL;
		}
		//delete point2d;
		myfile.close();
		split_polygons();
		sort_beziers();

		// Now extract the taxiway polygons for on_path_logic
		for (auto point2d : ALL) {
			if (point2d->get_type() == TAXIWAY) {
				std::vector<Point2> taxiway_polygon;
				for (auto seg : point2d->get_coordinates()) {
					taxiway_polygon.push_back(seg->pt);
				}
				taxiway_polygons.push_back(taxiway_polygon);
			}
		}

		return 1;
	}
	else
	{
		std::cout << "Unable to open file";
		return 0;
	}
}

void parseCpfFile(const std::string& filename, std::vector<Path>& paths) {
	std::ifstream file(filename);
	LAST_CPF_PATH = filename;

	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	std::string line;
	Path currentPath;

	while (std::getline(file, line)) {
		std::istringstream iss(line);

		// Check if the line starts with [PATH]
		if (line.find("[PATH ") != std::string::npos) {
			// New path starts
			if (!currentPath.name.empty()) {
				// Close the previous path before starting a new one
				paths.push_back(currentPath);
			}
			currentPath.points.clear();  // Clear previous path points
			iss >> line >> currentPath.name;  // Read path name
			currentPath.name.erase(currentPath.name.size() - 1); // Remove the trailing ']'
			iss >> currentPath.surfaceType;  // Read surface type
		}
		else if (!line.empty() && line.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
			// Whitespace line encountered, consider it as an indication to close the current path
			if (!currentPath.name.empty()) {
				paths.push_back(currentPath);
				currentPath.name.clear();
			}
		}
		else {
			// Read coordinate points
			Point2 coord;
			if (iss >> coord.y_ >> coord.x_) {
				currentPath.points.push_back(coord);
			}
		}
	}

	// Check if there's an unfinished path
	if (!currentPath.name.empty()) {
		paths.push_back(currentPath);
	}

	file.close();
}

void FileReader::LoadCollisionPaths(const std::string& filename) {

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		Point2 coord;

		// Assuming the format is "latitude,longitude"
		char delimiter;
		if (iss >> coord.y_ >> delimiter >> coord.x_) {
			clickPoints.push_back(coord);
		}
		else {
			std::cerr << "Error parsing line: " << line << std::endl;
		}
	}

	file.close();
}

void FileReader::DumpCollisionsToFile() {
	std::ofstream outputFile("collisionPoints.txt");
	if (!outputFile.is_open()) {
		MessageBox(NULL, L"Error opening file for writing!", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	outputFile << std::fixed << std::setprecision(6);

	for (const auto& point : clickPoints) {
		outputFile << point.y_ << " " << point.x_ << std::endl;
	}

	outputFile.close();
	MessageBox(NULL, L"Collision paths dumped to file!", L"Success", MB_OK | MB_ICONINFORMATION);

	// Clear the stored click points
	clickPoints.clear();
}

void sort_beziers() {
	for (size_t i = 0; i < ALL.size(); i++)
	{
		PointTess* point2d = ALL[i];
		int type = point2d->get_type();
		if (type != RUNWAY)
		{
			std::vector<LinearSegment*> segs = point2d->get_coordinates();
			std::vector<LinearSegment*> new_seg;

			for (auto it = segs.begin(); it != segs.end(); it++)
			{
				LinearSegment* cur = *it;

				bool has_lo = is_curved_node(*cur);
				Point2	lo_pt;
				if (has_lo) lo_pt = recip(cur->pt, cur->ctrl);

				std::vector<LinearSegment*>::iterator next = it, orig = it, prev = it;
				++next;
				while (next != segs.end() && ((LinearSegment*)*next)->pt == cur->pt &&
					(!is_curved_node(*((LinearSegment*)*prev)) || !is_curved_node(*((LinearSegment*)*next))) &&
					(is_curved_node(*((LinearSegment*)*prev)) || is_curved_node(*((LinearSegment*)*next))))
				{
					prev = next;

					++next;
				}
				--next;
				it = next;
				cur = *next;

				bool has_hi = is_curved_node(*cur);
				Point2 hi_pt;
				if (has_hi) hi_pt = cur->ctrl;

				bool is_split = (has_lo != has_hi || *orig != cur);
				cur->ctrl_hdl.split = is_split;

				if (has_lo) cur->ctrl_hdl.SetLo(lo_pt);
				if (has_hi) cur->ctrl_hdl.SetHi(hi_pt);


				//std::cout << cur->pt.y_ << ", " << cur->pt.x_ << ", " << is_split << std::endl;
				//if (has_lo) std::cout << "lo: " << lo_pt.y_ << ", " << lo_pt.x_ << ", " << is_split << std::endl;
				//if (has_hi) std::cout << "hi: " << hi_pt.y_ << ", " << hi_pt.x_ << ", " << is_split << std::endl;
				new_seg.push_back(cur);
			}


			point2d->set_coordinates(new_seg);

			//delete extra nodes
			auto nseg_it = new_seg.begin(), it2 = segs.begin();
			while (it2 != segs.end()) {
				if (*it2 == *nseg_it) {
					++it2;
					++nseg_it;
				}
				else {
					LinearSegment* it_seg = *it2;
					it2 = segs.erase(it2);
					delete it_seg;
				}
			}

			segs = point2d->get_coordinates();
			new_seg.clear();

			//final pass to build beiziers
			for (auto it = segs.begin(); it != segs.end(); it++)
			{
				LinearSegment* cur = *it;
				bool is_split = cur->ctrl_hdl.split;
				bool has_lo = cur->ctrl_hdl.has_lo, has_hi = cur->ctrl_hdl.has_hi;
				bool cur_curved = has_lo || has_hi;

				if (is_split)
				{
					if (has_lo)
					{
						LinearSegment* prev = get_prev(it, segs);
						bool prev_split = prev->ctrl_hdl.split;
						if (prev->ctrl_hdl.has_hi)
						{
							cubic_bezier(prev->pt, prev->ctrl_hdl.GetHi(), cur->ctrl_hdl.GetLo(), cur->pt, new_seg);
						}
						else
						{
							cubic_bezier(prev->pt, prev->pt, cur->ctrl_hdl.GetLo(), cur->pt, new_seg);
						}
					}

					new_seg.push_back(cur);

					if (has_hi)
					{
						LinearSegment* next = get_next(it, segs);

						bool next_split = next->ctrl_hdl.split;
						if (!next->ctrl_hdl.has_lo)
						{
							//TODO BUG!! JOIN TO LAST END GENERATED OF NEXT CURVE, RATHER THAN THE EXACT PT (LOCATION) OF THE LAST CURVE
							cubic_bezier(cur->pt, cur->ctrl_hdl.GetHi(), next->pt, next->pt, new_seg);
						}
					}
				}
				else
				{

					if (has_hi) {
						LinearSegment* prev = get_prev(it, segs);
						cubic_bezier(prev->pt, prev->pt, cur->ctrl_hdl.GetLo(), cur->pt, new_seg);
						new_seg.push_back(cur);
					}
					else
					{
						new_seg.push_back(cur);
					}
				}
			}

			point2d->set_coordinates(new_seg);
		}
	}
}

void split_polygons() {
	auto i = ALL.begin();
	std::vector<PointTess*> new_all;
	while (i != ALL.end())
	{
		PointTess* point2d = *i;
		int type = point2d->get_type();
		if (type != RUNWAY)
		{
			std::vector<LinearSegment*> segs = point2d->get_coordinates();

			point2d->remove_coordinates();
			new_all.push_back(point2d);
			for (auto it = segs.begin(); it != segs.end(); it++)
			{
				LinearSegment* cur = *it;

				if (is_closed(*cur) && (it + 1) != segs.end())
				{
					point2d->add_coord(cur);
					point2d = new PointTess(**i);
					point2d->remove_coordinates();
					new_all.push_back(point2d);
					continue;
				}
				point2d->add_coord(cur);
			}
		}
		else
		{
			new_all.push_back(point2d);
		}
		++i;
	}
	ALL = new_all;
}

