#ifndef MIRROR_H
#define MIRROR_H

#ifndef MIRROR_CLS_h
#define MIRROR_CLS_h
class Mirror;
class Mirror {
private:
	int x_, y_, width, height;
	double zoom_, lat_, lon_;
public:
	int borderDl, sectorDl;
	bool renderBorder;
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

#endif

