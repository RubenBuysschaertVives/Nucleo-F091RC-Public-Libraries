#include "stm32f091xc.h"
#include "timer17.h"

void InitTimer17(void)
{
	// Gebruik Timer 17 voor het genereren van een PWM-signaal.
	// Mik op 0,33...Hz frequentie en een resolutie van 3000.
	// Deze timer kan een PWM aanmaken voor pin PB9 of PA7.
	// Hier gebruiken we PB9...
	
	// PB9 moet TIM17_CH1 worden via alternate function 2.
	GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODER9) | GPIO_MODER_MODER9_1;
	GPIOB->AFR[1] |= 0x00000020;																							// Alternate function 2
	
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;																			// Clock voorzien voor de Timer17.	
	TIM17->PSC = ...; 																											// Prescaler op 1/48000 => 48000000/48000 => 1ms per puls
	TIM17->ARR = PWM_MAXIMUM - 1; 																						// Periode van 3s want: 1ms * 3000 = 3s => gewenste periode voor de knipper-LED.
	TIM17->CCR1 = 0; 																													// Aantijd voor OC1. Voorlopig 0% duty cycle.
	TIM17->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE; 		// PWM mode 1 op OC1/PB9, enable preload register op OC1 (OC1PE = 1).
	TIM17->CCER |= TIM_CCER_CC1E; 																						// Enable OC1 output.
	TIM17->BDTR &= ~TIM_BDTR_MOE; 																						// Disable output (MOE = 0) (optioneel).
	TIM17->CR1 |= TIM_CR1_CEN; 																								// Enable counter (CEN = 1).
	TIM17->EGR |= TIM_EGR_UG; 																								// Force update generation (UG = 1).
}

// Aantijd tussen 0 en 3000 instellen.
void SetPWMTimer17(uint16_t pwm)
{
	if(pwm > PWM_MAXIMUM)
		pwm = PWM_MAXIMUM;
	TIM17->CCR1 = pwm;																												// Aantijd voor OC1
	//TIM1->EGR |= TIM_EGR_UG;																								// Force update generation (UG = 1).
}

// PWM inschakelen.
void EnablePWMTimer17(void)
{
	// Enable output (MOE = 1).
	TIM17->BDTR |= TIM_BDTR_MOE; 																					
}

// PWM uitschakelen.
void DisablePWMTimer17(void)
{
	// Disnable output (MOE = 0).
	TIM17->BDTR &= ~TIM_BDTR_MOE;
}
