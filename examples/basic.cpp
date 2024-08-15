#include <albeRTOS.h>

void count_s() {
	int s = 0;

	while(true) {
		s += 1;
		albeRTOS::sleep(1000);
	}
}

void count_ms() {
	int ms = 0;

	while(true) {
		ms += 1;
		albeRTOS::sleep(1);
	}
}

int main() {

	albeRTOS::init();

	albeRTOS::addThread(count_s, 2, (char*)"Count seconds");
	// Give count_ms a higher priority since when t is a multiple of 1000 ms, 
	// count_ms should be updated first to maintain accuracy (I think? TODO) 
	albeRTOS::addThread(count_ms, 1, (char*)"Count milliseconds");

	albeRTOS::launch();

	// unreachable!
	while(true);

	return 0;
}

