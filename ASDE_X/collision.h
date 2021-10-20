#ifndef COLLISION_H
#define COLLISION_H

class Aircraft;

#include <unordered_map>
#include "aircraft.h"

#ifndef COLLISION_Collision_h
#define COLLISION_Collision_h
class Collision
{
private:
	Aircraft* aircraft1;
	Aircraft* aircraft2;
	bool render_flags[ACF_FLAG_COUNT];
	bool update_flags[ACF_FLAG_COUNT];
public:
	Collision(Aircraft* aircraft1, Aircraft* aircraft2);
	~Collision();
	unsigned int collLineDL = 0;
	bool getRenderFlag(int flag);
	void setRenderFlag(int flag, bool val);
	bool getUpdateFlag(int flag) { return update_flags[flag]; }
	void setUpdateFlag(int flag, bool val) { update_flags[flag] = val; }
	Aircraft* getAircraft1() { return aircraft1; }
	Aircraft* getAircraft2() { return aircraft2; }
};
#endif

extern std::unordered_map<std::string, Collision*> Collision_Map;
extern std::unordered_map<std::string, double**> runway_polygons;
extern std::vector<std::string> logic;

#endif

