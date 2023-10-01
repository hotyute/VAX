#pragma once

#include <vector>

struct LatLon {
    float lat, lon;
};

class ClosureArea {
public:
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
