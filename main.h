#include "stm32f091xc.h"
#include "stdbool.h"

#if !defined(MAIN_DEFINED)
	#define MAIN_DEFINED
	
	// Enumeratie aanmaken, om zo de woorden 'off' en 'on' om te zetten naar 'false' en 'true'.
	enum offOn {off = false, on = true};
	
	// Functie prototypes.
	void SystemClock_Config(void);
	void WaitForMs(uint32_t timespan);
#endif