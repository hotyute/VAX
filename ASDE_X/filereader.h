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

void open_adx(std::string path);

void sort_beziers();

void split_polygons();

extern std::string LAST_ADX_PATH, LAST_POF_PATH, LAST_ALIAS_PATH;
