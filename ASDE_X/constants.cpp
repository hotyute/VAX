#include "constants.h"

const char* PILOT_RATINGS[] = {
	"R1",
	"R2",
	"R3"
};
const int NUM_PILOT_RATINGS = sizeof(PILOT_RATINGS) / sizeof(PILOT_RATINGS[0]);

const char* CONTROLLER_RATINGS[] = {
	"OBS",
	"S1",
	"S2",
	"S3",
	"C1",
	"C2",
	"C3",
	"I1",
	"I2",
	"I3",
	"SUP",
	"ADM"
};
const int NUM_CONTROLLER_RATINGS = sizeof(CONTROLLER_RATINGS) / sizeof(CONTROLLER_RATINGS[0]);