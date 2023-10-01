#include "events.h"
#include "projection.h"
#include "packets.h"
#include "usermanager.h"
#include "renderer.h"

//event handler for position updates
void PositionUpdates::execute() {
	if (PositionUpdates::eAction.getTicks() == 0
		|| USER->getUpdateTime() != PositionUpdates::eAction.getTicks()) {
		PositionUpdates::eAction.setTicks(USER->getUpdateTime());
	}
	USER->setLatitude(CENTER_LAT);
	USER->setLongitude(CENTER_LON);
	sendPositionUpdates(*USER);
}


void PositionUpdates::stop() 
{
}


void ConfigUpdates::execute() {
	if (ConfigUpdates::eAction.getTicks() == 0
		|| USER->getUpdateTime() != ConfigUpdates::eAction.getTicks()) {
		ConfigUpdates::eAction.setTicks(1000);
	}
	/*Aircraft* acf = acf_map["EGF4427"];

	if ((acf->getHeading() + 3) > 360)
		acf->setHeading((acf->getHeading() + 3) - 360);
	else
	acf->setHeading(acf->getHeading() + 3);*/

	renderDate = true;
	renderCoordinates = true;
}

void ConfigUpdates::stop() 
{

}