#include "stm32f091xc.h"

#if !defined(TIMER1_DEFINED)
	#define TIMER1_DEFINED

	// PWM-tijden in µs.
	#define PWM_PERIOD 	20000
	#define PWM_MINIMUM	1000
	#define PWM_MAXIMUM	2000

	// Functieprototypes
	void InitTimer1(void);
	void SetPWMTimer1(uint16_t pwm);
	void EnablePWMTimer1(void);
	void DisablePWMTimer1(void);
#endif
