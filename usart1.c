#include "stm32f091xc.h"
#include "usart1.h"

void InitUsart1(uint32_t baudRate)
{
	// Clock voor GPIOA inschakelen.
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOAEN;
	
	// Pinnen van de USART2 instellen als alternate function.	
	// PA9 => TX-pin
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER9) | GPIO_MODER_MODER9_1;		// Alternate function op PA2
	GPIOA->AFR[1] |= 0x00000010;																								// USART2_TX is alternate function AF1 (zie datasheet STM32F091RC, p43).

	// PA10 => RX-pin
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER10) | GPIO_MODER_MODER10_1;		// Alternate function op PA3
	GPIOA->AFR[1] |= 0x00000100;
		
	// Usart module van een klok voorzien
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	
	// Baudrate zetten
	USART1->BRR = 48000000/baudRate;
	
	// Reception overrun disable. Als nieuwe byte aankomt terwijl
	// vorige nog niet verwerkt is, geen overrun error genereren.
	USART1->CR3 |= USART_CR3_OVRDIS;
	
	// USART inschakelen
	USART1->CR1 |= USART_CR1_UE;
	
	// Transmitter enable
	USART1->CR1 |= USART_CR1_TE;
	
//	// Interrupt voor de receiver enable (receiver buffer not empty interrupt enable)
//	USART1->CR1 |= USART_CR1_RXNEIE;
//	
//	// Koppeling van interrupt maken met de NVIC
//	NVIC_SetPriority(USART1_IRQn, 0);
//	NVIC_EnableIRQ(USART1_IRQn);
//	
//	// Receiver enable
//	USART1->CR1 |= USART_CR1_RE;
}

void StringToUsart1(char* string)
{
	uint8_t indexer = 0;
	
	while(string[indexer] != 0)
	{
		// Byte versturen.
		USART1->TDR = (uint8_t)string[indexer++];
		
		// Wachten tot byte vertuurd is.
		while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC);
	}
}
