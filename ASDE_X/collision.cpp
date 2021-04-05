#include "collision.h"

std::unordered_map<std::string, Collision*> Collision_Map;

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
