#include "tempdata.h"

std::vector<ClosureArea> closureAreas;

void startNewClosureArea() {
    ClosureArea newArea;
    closureAreas.push_back(newArea);
}

void addPointToActiveArea(float lat, float lon) {
    if (!closureAreas.empty()) {
        closureAreas.back().addPoint(lat, lon);
    }
}

void finishDefiningArea() {
    // Any post-processing or validation for the current area can be done here
}