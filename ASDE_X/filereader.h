#pragma once

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <sstream>
#include <iomanip>
#include "renderer.h"
#include "point2d.h"
#include "projection.h"
#include "dxfdrawing.h"
#include "tools.h"

struct Path {
	std::string name;
	int surfaceType;
	std::vector<Point2> points;
};

class FileReader {
public:
	static int LoadADX(std::string);
	std::vector<Path> collisionPaths;
	std::vector<Point2> clickPoints;
	void LoadCollisionPaths(const std::string& path);
	void DumpCollisionsToFile();
};

void open_adx(std::string path);

void parseCpfFile(const std::string& filename, std::vector<Path>& paths);

void sort_beziers();

void split_polygons();

extern std::string LAST_ADX_PATH, LAST_POF_PATH, LAST_ALIAS_PATH, LAST_CPF_PATH;
