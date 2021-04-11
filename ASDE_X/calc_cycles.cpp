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
		long long time1 = 30L;
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
			// iterator->first = key
			Aircraft* acf = iter->second;
		}
	}

	if (Collision_Map.size() > 0) {
		for (auto iter = Collision_Map.begin(); iter != Collision_Map.end(); iter++) {
			// iterator->first = key
			Collision* col = iter->second;
		}
	}
}
