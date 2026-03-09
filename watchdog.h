#include "stm32f091xc.h"

#if !defined(WATCHDOG_DEFINED)
	#define WATCHDOG_DEFINED
	
	void InitWatchdog(void);
	void ResetWatchdog(void);
#endif
