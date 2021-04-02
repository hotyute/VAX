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

std::vector<Mirror*>::iterator get_mir(std::vector<Mirror*> &vec, std::string search)
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
