#ifndef COLLISION_H
#define COLLISION_H

#include <unordered_map>
#include "aircraft.h"

#ifndef COLLISION_Collision_h
#define COLLISION_Collision_h
class Collision
{
private:
	Aircraft* aircraft1;
	Aircraft* aircraft2;
public:
	Collision(Aircraft* aircraft1, Aircraft* aircraft2);
	~Collision();
};
#endif

extern std::unordered_map<std::string, Collision*> Collision_Map;

#endif

