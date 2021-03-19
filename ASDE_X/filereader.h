#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "renderer.h"

class FileReader {
public:
	static int LoadADX(std::string);
};

#endif

void sort_beziers();

void split_polygons();
