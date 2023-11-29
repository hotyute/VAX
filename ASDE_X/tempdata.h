#pragma once

#include <vector>
#include <string>
#include <any>

struct LatLon {
    float lat, lon;

    bool operator!=(const LatLon& other) const {
        return lat != other.lat || lon != other.lon;
    }
};

class LineVis {
public:
    void addPoint(float lat, float lon) { points.push_back({ lat, lon }); }

    const std::vector<LatLon>& getPoints() const { return points; }

    void clear() { points.clear(); }

    void pop_back() { points.pop_back(); }
private:
    std::vector<LatLon> points;
};

struct ClientScript {
    int idx = -1;
    ClientScript(std::string assem) : assembly(assem) {};
    std::string assembly;
    std::vector<std::any> objects;
};

class ClosureArea : public ClientScript {
public:
    bool opened = false;
    void addPoint(float lat, float lon) {
        points.push_back({ lat, lon });
    }

    void transfer(std::vector<LatLon>& source) {
        points = std::move(source);
    }

    const std::vector<LatLon>& getPoints() const {
        return points;
    }

private:
    std::vector<LatLon> points;
};

extern LineVis debug_vis;
extern std::vector<ClosureArea*> closureAreas;
