#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

class FileReader {
public:
	static int LoadADX(std::string);
};

#endif