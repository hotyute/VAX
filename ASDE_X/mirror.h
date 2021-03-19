#ifndef MIRROR_H
#define MIRROR_H

#include <string>
#include <vector>
#include <unordered_map>

#ifndef MIRROR_CLS_h
#define MIRROR_CLS_h
class Mirror;
class Mirror {
private:
	int x_, y_, width, height;
	double zoom_, lat_, lon_;
public:
	std::string id_;
	int borderDl, sectorDl, aircraftDl, heavyDl, unkTarDl;
	bool renderBorder, renderSector, renderAircraft;
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

#endif

