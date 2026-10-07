#include "stm32f091xc.h"
#include "timer1.h"

void InitTimer1(void)
{
	// Gebruik Timer 1 voor het genereren van een PWM-signaal. Mik op 50Hz frequentie en een resolutie van 1000.
	// Je kan PWM aanmaken voor pin PA8 en PA9. PA8 is verbonden met LED6 van het Nucleo Extension shield,
	// PA9 met een servomotor (gekoppeld aan PA9 dus).
	// Op die manier kan je ook visueel de PWM iet-of-wat controleren via LED6.

	// PA8/LED6 moet TIM1_CH1 worden via alternate function 2.
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER8) | GPIO_MODER_MODER8_1;		// Alternate function op PA8.
	GPIOA->AFR[1] |= 0x00000002;		// Alternate function 2.
	
	// PA9/modelbouw ESC moet TIM1_CH2 worden via alternate function 2.
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER9) | GPIO_MODER_MODER9_1;		// Alternate function op PA9.
	GPIOA->AFR[1] |= 0x00000020;		// Alternate function 2.
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;			// Clock voorzien voor de timer1.	
	TIM1->PSC = 48 - 1;											// Prescaler op 1/48 => 48000000/48 => 1 µs per puls.
	TIM1->ARR = PWM_PERIOD;									// Periode van 20 ms want: 1µs * 20000 = 20ms of  = 1/48000000 * 48 * 20000 = 20ms.
	TIM1->CCR1 = PWM_MINIMUM; 							// Aantijd voor OC1 instellen.
	TIM1->CCR2 = PWM_MINIMUM; 							// aantijd voor OC2 instellen.
	TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE; 	// PWM mode 1 op OC1/PA8, enable preload register op OC1 (OC1PE = 1)
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE; 	// PWM mode 1 op OC2/PA9, enable preload register op OC2 (OC2PE = 1)
	TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E; 														// Enable OC1 (en OC2) output.
	TIM1->BDTR &= ~TIM_BDTR_MOE; 																						// Disable output (MOE = 0).
	TIM1->CR1 |= TIM_CR1_CEN; 																							// Enable counter (CEN = 1).
	TIM1->EGR |= TIM_EGR_UG; 																								// Force update generation (UG = 1).
}

// PWM aantijd instellen (tussen 1000 en 2000µs).
void SetPWMTimer1(uint16_t pwm)
{
	if((pwm >= PWM_MINIMUM) && (pwm <= PWM_MAXIMUM))
	{
		TIM1->CCR1 = pwm; 										// Aantijd voor OC1.
		TIM1->CCR2 = pwm; 										// aantijd voor OC2.
	}
	else
	{
		TIM1->CCR1 = 0; 											// PWM op 0% duty cycle zetten.
		TIM1->CCR2 = 0; 											// PWM op 0% duty cycle zetten.
	}
}

// Globale PWM-uitgangen inschakelen.
void EnablePWMTimer1(void)
{	
	// Enable output (MOE = 1).
	TIM1->BDTR |= TIM_BDTR_MOE; 																					
}

// Globale PWM-uitgangen uitschakelen.
void DisablePWMTimer1(void)
{
	// Disnable output (MOE = 0).
	TIM1->BDTR &= ~TIM_BDTR_MOE; 																					
}
