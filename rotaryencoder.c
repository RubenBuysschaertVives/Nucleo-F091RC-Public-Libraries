#include "stm32f091xc.h"
#include "rotaryencoder.h"
#include "stdbool.h"

// Omdat deze variabele niet 'static' staat gedefinieer, kan ze ook in een ander bestand opgeroepen worden.
// Dat doe je dan via het keyword 'extern'... Probeer onderstaande maar eens te gebruiken in de main.c...
volatile uint8_t rotaryEncoderCounter = 0;

void InitRotaryEncoder(void)
{
	// Clock voor GPIOA inschakelen.
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOAEN;	
	
	// LET OP: PA5 is gekoppeld met de user LED (LD2). Dus voor de goede werking
	// zou je jumper SB21 moeten verwijderen op de achterkant van het Nucleo-F091RC board ...
	#if defined(ROTARY_ENCODER_USE_SWITCH)
		// Drukknop is gekoppeld met PA5. Zet die op input.
		GPIOA->MODER = GPIOA->MODER & ~GPIO_MODER_MODER5;
	#endif
	
	// Encoder signaal A is gekoppeld met PA6. Zet die op input.
	GPIOA->MODER = GPIOA->MODER & ~GPIO_MODER_MODER6;
	
	// Encoder signaal B is gekoppeld met PA7. Zet die op input.
	GPIOA->MODER = GPIOA->MODER & ~GPIO_MODER_MODER7;
	
	// Indien je met interrupts wil werken, stel alles zo in.
	// Maak die keuze in 'rotaryencoder.h'.
	#if defined(ROTARY_ENCODER_USE_INTERRUPTS)
		// SYSCFG clock enable.
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
		
		// PA6 koppelen aan EXTI 6.
		SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PA;
		
		// Rising edge detecteren.
		EXTI->RTSR = EXTI->RTSR | EXTI_RTSR_TR6;
		
		// Interrupt toelaten
		EXTI->IMR = EXTI->IMR | EXTI_IMR_MR6;
		
		// Eén van de 4 prioriteiten kiezen.
		NVIC_SetPriority(EXTI4_15_IRQn,0);
		
		// Interrupt effectief toelaten.
		NVIC_EnableIRQ(EXTI4_15_IRQn);
	#endif
}

// Functie om de toestand van de drukknop in de encoder op te meten.
#if defined(ROTARY_ENCODER_USE_SWITCH)
	bool RotaryEncoderSwitchActive(void)
	{
		// Rotary encoder drukknop actief?
		if((GPIOA->IDR & GPIO_IDR_5) != GPIO_IDR_5)
				return true;
		else
				return false;
	}
#endif

// Als je geen interrupts gebruikt, maak dan een functie om de toestand van signaal A op te meten.
#if !defined(ROTARY_ENCODER_USE_INTERRUPTS)
	bool RotaryEncoderAActive(void)
	{
		// Rotary encoder drukknop actief?
		if((GPIOA->IDR & GPIO_IDR_6) == GPIO_IDR_6)
				return true;
		else
				return false;
	}
#endif

// Functie om de toestand van signaal B op te meten.
bool RotaryEncoderBActive(void)
{
	// Rotary encoder drukknop actief?
	if((GPIOA->IDR & GPIO_IDR_7) == GPIO_IDR_7)
			return true;
	else
			return false;
}

// Werk je met interrupts? Verwerk de stijgende flank van signaal A dan hieronder...
#if defined(ROTARY_ENCODER_USE_INTERRUPTS)
	// Interrupt handler aanmaken.
	void EXTI4_15_IRQHandler(void) 
	{	
		// Als het een interrupt is van PA6, signaal A?
		if((EXTI->PR & EXTI_PR_PR6) == EXTI_PR_PR6)
		{
			// Interrupt (pending) vlag wissen door er een 1 naar te schrijven.
			EXTI->PR |= EXTI_PR_PR6;
			
			// Kijken hoe het zit met signaal B. Aan de hand daarvan kan je weten of 
			// je links of rechts draaide.
			// Pas de globale variabele aan volgens de huidige situatie...
			if(RotaryEncoderBActive())
			{
				if(rotaryEncoderCounter > 0)
					rotaryEncoderCounter--;			// Tegenwijzerzin
			}
			else
			{
				if(rotaryEncoderCounter < 255)
					rotaryEncoderCounter++;			// Wijzerzin
			}
		}
	}
#endif
