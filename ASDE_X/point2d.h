#ifndef POINT2D_H
#define POINT2D_H

#include <vector>

#define RUNWAY 11451
#define TAXIWAY 11452
#define PARKING 11453
#define APRON 11454

struct	Point2;
struct	Vector2;

enum class LINE_TYPE { NODE, NODE_CTRL};

enum class LOOP_TYPE { LOOP, CLOSE, END };

struct Point2 {
	Point2() : x_(0.0), y_(0.0), p{ 0.0, 0.0, 0.0 } { }
	Point2(double ix, double iy) : x_(ix), y_(iy), p{ ix, iy, 0.0 } { }
	Point2(const Point2& rhs) : x_(rhs.x_), y_(rhs.y_), p{ rhs.x_, rhs.y_, 0.0 } { }

	~Point2() { }

	Point2& operator=(const Point2& rhs) { x_ = rhs.x_; y_ = rhs.y_; return *this; }
	bool operator==(const Point2& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }
	bool operator!=(const Point2& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }

	Point2& operator += (const Vector2& v);
	Point2& operator -= (const Vector2& v);
	Point2 operator+(const Vector2& v) const;
	Point2 operator-(const Vector2& v) const;

	bool unset() { return x_ == -1 && y_ == -1; }

	double* as_array();

	double x_, y_;
	double p[3];
};

struct Vector2 {
	Vector2() : dx(0.0), dy(0.0) { }
	Vector2(double ix, double iy) : dx(ix), dy(iy) { }
	Vector2(const Vector2& rhs) : dx(rhs.dx), dy(rhs.dy) { }
	explicit Vector2(const Point2& rhs) : dx(rhs.x_), dy(rhs.y_) { }
	Vector2(const Point2& p1, const Point2& p2) : dx(p2.x_ - p1.x_), dy(p2.y_ - p1.y_) { }

	Vector2& operator=(const Vector2& rhs) { dx = rhs.dx; dy = rhs.dy; return *this; }
	bool operator==(const Vector2& rhs) const { return dx == rhs.dx && dy == rhs.dy; }
	bool operator!=(const Vector2& rhs) const { return dx != rhs.dx || dy != rhs.dy; }

	Vector2& operator += (const Vector2& rhs) { dx += rhs.dx; dy += rhs.dy; return *this; }
	Vector2& operator -= (const Vector2& rhs) { dx -= rhs.dx; dy -= rhs.dy; return *this; }
	Vector2& operator *= (double scalar) { dx *= scalar; dy *= scalar; return *this; }
	Vector2& operator/= (double scalar) { dx /= scalar; dy /= scalar; return *this; }
	Vector2	 operator+ (const Vector2& v) const { return Vector2(dx + v.dx, dy + v.dy); }
	Vector2	 operator- (const Vector2& v) const { return Vector2(dx - v.dx, dy - v.dy); }
	Vector2  operator* (double scalar) const { return Vector2(dx * scalar, dy * scalar); }
	Vector2  operator/ (double scalar) const { return Vector2(dx / scalar, dy / scalar); }
	Vector2	 operator- (void) const { return Vector2(-dx, -dy); }

	double dx, dy;
};

struct HiLo {
	Point2 hi;
	Point2 lo;

	bool split, has_hi, has_lo;

	void SetLo(Point2 f) { lo = Point2(f.x_, f.y_); has_lo = true; }
	void SetHi(Point2 f) { hi = Point2(f.x_, f.y_); has_hi = true; }

	Point2& GetLo() { return lo; }
	Point2& GetHi() { return hi; }
};

struct	LinearSegment {
	~LinearSegment() { }
	LINE_TYPE	type;
	LOOP_TYPE	loop_type;
	Point2		pt;
	Point2		ctrl;

	HiLo ctrl_hdl;
};

// These must be defined below because Point2 is declared before Vector2.
inline Point2& Point2::operator += (const Vector2& v) { x_ += v.dx; y_ += v.dy; return *this; }
inline Point2& Point2::operator -= (const Vector2& v) { x_ -= v.dx; y_ -= v.dy; return *this; }
inline Point2 Point2::operator+(const Vector2& v) const { return Point2(x_ + v.dx, y_ + v.dy); }
inline Point2 Point2::operator-(const Vector2& v) const { return Point2(x_ - v.dx, y_ - v.dy); }

class PointTess {
private:
	int type;
	std::vector<LinearSegment*> coordinates;
	std::vector<LinearSegment*> holes;
public:
	~PointTess();
	void set_type(int t) {
		type = t;
	}
	int get_type() { return type; }
	void add_coordinates(LinearSegment*p);
	Point2 add_coordinates(double lat, double lon, double z);
	void add_vector(double lat, double lon, double z, double lat2, double lon2, double z2);
	void add_holes(LinearSegment* p);
	void add_coord(LinearSegment* p);
	void add_holes(double x, double y, double z);
	std::vector<LinearSegment*> get_coordinates();
	void set_coordinates(std::vector<LinearSegment*> new_seg);
	void remove_coordinates();
	LinearSegment* get_last_coordinate();
	std::vector<LinearSegment*> get_holes();
};

extern std::vector<PointTess*> ALL;

#endif

