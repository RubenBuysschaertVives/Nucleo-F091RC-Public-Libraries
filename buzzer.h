#include "stm32f091xc.h"
#include "stdbool.h"

#if !defined(BUZZR_DEFINED)
	#define BUZZER_DEFINED
	
	void InitBuzzer(void);
	void Buzzer(bool offOn);
#endif
