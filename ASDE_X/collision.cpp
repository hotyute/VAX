#include "collision.h"

std::unordered_map<std::string, Collision*> Collision_Map;
std::unordered_map<std::string, double**> runway_polygons;
std::vector<std::string> logic;

Collision::Collision(Aircraft* aircraft1, Aircraft* aircraft2)
{
	Collision::aircraft1 = aircraft1;
	Collision::aircraft2 = aircraft2;
}

Collision::~Collision()
{
	Collision::aircraft1 = nullptr;
	Collision::aircraft2 = nullptr;
}

bool Collision::getRenderFlag(int flag)
{
	return Collision::render_flags[flag];
}

void Collision::setRenderFlag(int flag, bool val)
{
	Collision::render_flags[flag] = val;
}
