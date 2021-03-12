#include <vector>

#define RUNWAY 11451
#define TAXIWAY 11452
#define PARKING 11453
#define APRON 11454

class Point2D {
private:
	int type;
	std::vector<double*> coordinates;
	std::vector<double*> holes;
public:
	void set_type(int t) {
		type = t;
	}
	int get_type() { 
		return type; 
	}
	void add_coordinates(double*);
	void add_holes(double*);
	std::vector<double*> get_coordinates();
	std::vector<double*> get_holes();
};

extern std::vector<Point2D*> ALL;