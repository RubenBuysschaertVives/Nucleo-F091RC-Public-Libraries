#include "stm32f091xc.h"

#if !defined(TIMER17_DEFINED)
	#define TIMER17_DEFINED
	
	#define PWM_MAXIMUM ...

	void InitTimer17(void);
	void SetPWMTimer17(uint16_t pwm);
	void EnablePWMTimer17(void);
	void DisablePWMTimer17(void);
#endif
