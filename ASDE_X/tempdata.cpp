#include "tempdata.h"

std::vector<ClosureArea> closureAreas;

void startNewClosureArea() {
	ClosureArea newArea;
	newArea.opened = true;
	closureAreas.push_back(newArea);
}

void addPointToActiveArea(float lat, float lon) {
	if (!closureAreas.empty()) {
		closureAreas.back().addPoint(lat, lon);
	}
}

void removePointFromActiveArea() {
	if (!closureAreas.empty()) {
		closureAreas.pop_back();
	}
}

void finishDefiningArea() {
	// Any post-processing or validation for the current area can be done here
	closureAreas.back().opened = false;
}