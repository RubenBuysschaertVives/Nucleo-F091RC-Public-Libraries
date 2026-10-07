#include "stm32f091xc.h"
#include "stdbool.h"

// Functie om de zoemer-pin op uitgang te zetten.
void InitBuzzer(void)
{
	// Clock voor GPIOA inschakelen.
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOAEN;
	
	// PA9 (TX1) op output zetten.
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER9) | GPIO_MODER_MODER9_0;
}

// De zoemer aansturen.
void Buzzer(bool offOn)
{
	if(offOn)
		GPIOA->BSRR |= GPIO_BSRR_BS_9;
	else
		GPIOA->BSRR |= GPIO_BSRR_BR_9;
}
