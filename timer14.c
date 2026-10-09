#include "stm32f091xc.h"
#include "timer14.h"
#include "leds.h"

// Variabelen maken, die ook via 'extern' in andere bestanden opgeroepen kunnen worden.
volatile bool newCapture = false;
volatile uint16_t distance = 0;

// Timer 14 instellen voor input capture. Dit voorbeeld werd gemaakt
// om vlot de ultrasone sensor HC-SR04 uit te lezen...
// Let op: deze sensor werkt op 5V! De microcontroller op 3,3V.
void InitTimer14(void)
{
	// Klok voorzien voor Timer 14.
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	
	// LET OP: onderstaande pin is NIET 5V-tolerant! Gebruik een spanningsdeler!
	// PA7 = HC-SR04 echo = TIM14_CH1 instellen als alternate function.
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER7) | GPIO_MODER_MODER7_1;
	// TIM14_CH1 is alternate function AF4.
	GPIOA->AFR[0] |= 0x40000000;
	
	// CC1 kanaal als input zetten. IC1 wordt gelinkt aan TI1 (zie blokschema).
	TIM14->CCMR1 |= TIM_CCMR1_CC1S_0;
	
//	// Input filter instellen (op klok/32 en 8 stabiele metingen).
//	// Op die manier kan je (een stuk) dender wegwerken (optioneel bij de HC-SR04).
//	TIM14->CCMR1 |= TIM_CCMR1_IC1F;	
	
	// Input capture prescaler instellen (op no prescaler).
	// Capture laten reageren op iedere ingestelde flank.
	TIM14->CCMR1 &= ~TIM_CCMR1_IC1PSC;
	
//	// Meting gevoelig aan dalende flank.
//	TIM14->CCER &= ~TIM_CCER_CC1NP;
//	TIM14->CCER |= TIM_CCER_CC1P;
	
	// Meting gevoelig aan stijgende flank (om de meting te starten).
	TIM14->CCER &= ~TIM_CCER_CC1P & ~TIM_CCER_CC1NP;

//	// Meting gevoelig aan stijgende en dalende flank.
//	TIM14->CCER = TIM_CCER_CC1P | TIM_CCER_CC1NP;
	
	// Capture enable.
	TIM14->CCER |= TIM_CCER_CC1E;
	
	// Interrupts toelaten.
	TIM14->DIER |= TIM_DIER_CC1IE;
	
	// Prescaler instellen om rechtstreeks te meten in centimeters...
	// 1cm (moet dubbel afgelegd worden) = 0.02m/340m/s = ~59 µs per centimeter.
	// Gemeten met een klok van 48MHz is dit 59µs/20,8ns = ~2832 stappen
	TIM14->PSC = 2832 - 1;
	
	// Teller inschakelen.
	TIM14->CR1 |= TIM_CR1_CEN;

	// Koppeling van interrupt maken met de NVIC.
	NVIC_SetPriority(TIM14_IRQn, 0);
	NVIC_EnableIRQ(TIM14_IRQn);
}

// Interrupt van Timer 14 verwerken (input capture).
void TIM14_IRQHandler(void)
{
	// Lokale variabele.
	uint16_t temp = 0; 
	
	// Interrutpt van Timer 14?
	if((TIM14->SR & TIM_SR_CC1IF) == TIM_SR_CC1IF)
	{
		// Capture compare interruptvlag resetten door het CCR1-register uit te lezen.
		temp = TIM14->CCR1;
		
		// Toestand van Echo pin bekijken (via de spanningsdeler).
		if((GPIOA->IDR & GPIO_IDR_7) == GPIO_IDR_7)
		{			
			// Stijgende flank gehad? De teller resetten voor de start van de meting.
			TIM14->CNT = 0;

			// Meting nu gevoelig maken aan dalende flank (om straks de meting te kunnen afronden).
			TIM14->CCER &= ~TIM_CCER_CC1NP;
			TIM14->CCER |= TIM_CCER_CC1P;
			
			// Echotijd visualiseren (LED laten oplichten).
			SetUserLed(true);
		}
		else
		{
			// Dalende flank gehad? De teller nu uitlezen en via hulpvariabele melden aan 
			// de hoofdlus.	
			distance = temp;
			newCapture = true;		

			// Echotijd visualiseren (LED doven).
			SetUserLed(false);
			
			// Meting terug gevoelig maken aan de stijgende flank.
			TIM14->CCER &= ~TIM_CCER_CC1P & ~TIM_CCER_CC1NP;
		}
	}
}
