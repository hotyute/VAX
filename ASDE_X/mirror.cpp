#include "mirror.h"

#include <boost/algorithm/string.hpp>

Mirror::Mirror()
{
	for (size_t i = 0; i < MIR_FLAG_COUNT; i++) {
		update_flags[i] = false;
		render_flags[i] = false;
	}
}

Mirror::~Mirror()
{
	g_flags.clear();
	c_flags.clear();
	for (auto& coord : wndc)
	{
		delete coord.second;
	}
	wndc.clear();
}

int Mirror::getWidth()
{
	return Mirror::width;
}

int Mirror::getHeight()
{
	return Mirror::height;
}

double Mirror::getZoom()
{
	return Mirror::zoom_;
}

void Mirror::setBoundaries(double minX, double maxX, double minY, double maxY)
{
	this->minX = minX;
	this->minY = minY;
	this->maxX = maxX;
	this->maxY = maxY;
}

bool Mirror::within_boundary(Aircraft& aircraft)
{
	if (aircraft.getLatitude() < minY || aircraft.getLatitude() > maxY)
		return false;
	if (aircraft.getLongitude() < minX || aircraft.getLongitude() > maxX)
		return false;
	return true;
}

void Mirror::handleMovement(double n_x, double n_y)
{
	double c_x = x_, c_y = y_;
	if (c_x != n_x || c_y != n_y)
	{// if moved
		update_flags[MIR_COLLISION_LINE] = true;
		update_flags[MIR_VECTOR] = true;
	}
	x_ = n_x;
	y_ = n_y;
}

std::vector<Mirror*>::iterator get_mir(std::vector<Mirror*>& vec, std::string search)
{
	auto it = std::find_if(
		std::begin(vec),
		std::end(vec),
		[&search](const auto& str) { return boost::iequals(search, ((Mirror&)str).id_); }
	);
	return std::vector<Mirror*>::iterator();
}

std::unordered_map<std::string, Mirror*>::iterator get_mir(std::unordered_map<std::string, Mirror*>& vec, std::string search)
{
	auto it = std::find_if(
		std::begin(vec),
		std::end(vec),
		[&search](const auto& str) { return boost::iequals(search, ((std::string)str.first)); }
	);
	return it;
}

std::unordered_map<Aircraft*, std::vector<unsigned int>>::iterator get_flags(std::unordered_map<Aircraft*, std::vector<unsigned int>>& vec, Aircraft* search)
{
	auto it = std::find_if(
		std::begin(vec),
		std::end(vec),
		[&search](const auto& str) { return str.first == search; }
	);
	return it;
}
