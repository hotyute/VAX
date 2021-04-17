#ifndef FILEREADER_H
#define FILEREADER_H

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

class FileReader {
public:
	static int LoadADX(std::string);
};

#endif

void sort_beziers();

void split_polygons();
