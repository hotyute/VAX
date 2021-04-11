#include "calc_cycles.h"

DWORD __stdcall CalcThread1(LPVOID)
{
	boost::posix_time::ptime start;
	boost::posix_time::ptime end;
	boost::posix_time::time_duration time;

	while (true)
	{
		start = boost::posix_time::microsec_clock::local_time();

		//code here
		update();
		CalculateCollisions();

		end = boost::posix_time::microsec_clock::local_time();

		time = (end - start);
		long long time1 = 100L;
		long long time2 = time1 - time.total_milliseconds();
		if (time2 < 1) {
			time2 = 1;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(time2));

	}
	return 0;
}

void update()
{
}

void CalculateCollisions() {
	if (AcfMap.size() > 0) {
		for (auto iter = AcfMap.begin(); iter != AcfMap.end(); iter++) {
			Aircraft* acf1 = iter->second;
			if (acf1) {
				Aircraft& aircraft1 = *acf1;
				for (auto iter2 = AcfMap.begin(); iter2 != AcfMap.end(); iter2++) {
					Aircraft* acf2 = iter2->second;
					if (acf2 && acf2 != acf1) {
						Aircraft& aircraft2 = *acf2;

						//Check What Runway Aircraft1
					}
				}
			}
		}
	}

	if (Collision_Map.size() > 0) {
		for (auto iter = Collision_Map.begin(); iter != Collision_Map.end(); iter++) {
			// iterator->first = key
			Collision* col = iter->second;
		}
	}
}
