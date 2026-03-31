#include "stm32f091xc.h"

#if !defined(USART1_DEFINED)
	#define USART1_DEFINED
	
//	enum UsartState {idle, busyReceiving, newStringArrived, receptionTimeOut, overflowOccured};
//	#define MAX_NUMBER_OF_CHARS 200

	void InitUsart1(uint32_t baudRate);	
	void StringToUsart1(char* string);
#endif


/*

Usage in main.c

void USART1_IRQHandler(void)
{
	if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		// Byte ontvangen, lees hem om alle vlaggen te wissen.
		uint8_t temp = USART1->RDR;		
		
		...
	}
}

*/
