#ifndef MIRROR_H
#define MIRROR_H

#include <string>
#include <vector>
#include <unordered_map>

#include "aircraft.h"
#include "collision.h"

#ifndef MIRROR_CLS_h
#define MIRROR_CLS_h
class Mirror;
class Mirror {
private:
	int x_, y_, width, height;
	double zoom_, lat_, lon_;
public:
	Mirror();
	~Mirror();
	std::string id_;
	std::unordered_map<Aircraft*, std::vector<unsigned int>> g_flags;
	std::unordered_map<Collision*, std::vector<unsigned int>> c_flags;
	POINT* s_pt = nullptr, * cur_pt = nullptr, * end_pt = nullptr;
	int startX = -1, startY = -1;
	int borderDl;
	bool renderBorder;
	bool update_flags[MIR_FLAG_COUNT];
	bool render_flags[MIR_FLAG_COUNT];
	int getX() { return x_; }
	int getY() { return y_; }
	void setX(int x) { x_ = x; }
	void setY(int y) { y_ = y; }
	void setWidth(int w) { width = w; }
	void setHeight(int h) { height = h; }
	int getWidth();
	int getHeight();
	void setZoom(double zoom) { zoom_ = zoom; }
	double getZoom();
	void setLat(double lat) { lat_ = lat; }
	void setLon(double lon) { lon_ = lon; }
	double getLat() { return lat_; }
	double getLon() { return lon_; }
};
#endif

extern std::vector<Mirror*>::iterator get_mir(std::vector<Mirror*>& vec, std::string search);
extern std::unordered_map<std::string, Mirror*>::iterator get_mir(std::unordered_map<std::string, Mirror*>& vec, std::string search);
extern std::unordered_map<Aircraft*, std::vector<unsigned int>>::iterator get_flags(std::unordered_map<Aircraft*, std::vector<unsigned int>>& vec, Aircraft* search);

#endif

