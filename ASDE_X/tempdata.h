#pragma once

#include <vector>

struct LatLon {
    float lat, lon;

    bool operator!=(const LatLon& other) const {
        return lat != other.lat || lon != other.lon;
    }
};

class LineVis {
public:
    void addPoint(float lat, float lon) {
        points.push_back({ lat, lon });
    }

    const std::vector<LatLon>& getPoints() const {
        return points;
    }

    void clear() {
        points.clear();
    }
private:
    std::vector<LatLon> points;
};

class ClosureArea {
public:
    bool opened = false;
    void addPoint(float lat, float lon) {
        points.push_back({ lat, lon });
    }

    const std::vector<LatLon>& getPoints() const {
        return points;
    }

private:
    std::vector<LatLon> points;
};

extern std::vector<ClosureArea> closureAreas;

void startNewClosureArea();

void addPointToActiveArea(float lat, float lon);

void finishDefiningArea();
