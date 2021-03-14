#ifndef MIRROR_H
#define MIRROR_H

#ifndef MIRROR_CLS_h
#define MIRROR_CLS_h
class Mirror;
class Mirror {
private:
	int x, y, width, height, zoom;
public:
	int getX();
	int getY();
	int getWidth();
	int getHeight();
	int getZoom();
};
#endif

#endif

