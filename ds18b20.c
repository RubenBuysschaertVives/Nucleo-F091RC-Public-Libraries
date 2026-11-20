#include "stm32f091xc.h"
#include "ds18b20.h"

// Deze code is gemaakt voor gebruik van een DS18B20-sensor, aangesloten op
// PA10 (pin 4 van de 10-pin header op het Nucleo Extension Shield).
// Wanneer er een ander pin gebruikt werd tijdens de verbinding, moet 
// in deze code hier en daar een aanpassing gedaan worden.

// Return't 1 indien DS18B20 gevonden...
uint8_t ResetDS18B20(void)
{
	// PA10, one-wire-bus, meer dan 480µs laag maken door master. 
	OneWireLow();
	OneWireAsOutput();
	WaitForMicroseconds(1000);
	OneWireAsInput();
	WaitForMicroseconds(75);
	if(OneWireReadBit() > 0)			// Is de 'presence' puls gedetecteerd?
	{
		WaitForMicroseconds(500);		// Nog wachten tot totaal minimum 480µs (75 + 500 > 480µs)
		return 0;
	}
	else
	{
		WaitForMicroseconds(500);
		return 1;
	}
}

void InitDS18B20(void)
{
	// Clock voor GPIOA inschakelen (PA10 = one-wire bus).
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOAEN;
	
	// Eventueel Pull-up voorzien (geldt zowel voor input als voor output (in open drain) mode)
	// als die nog niet hardwarematig voorzien werd.
	// GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_0;
	
	if(ResetDS18B20())
	{
		OneWireWriteByte(0xCC);		// Skip ROM.
		OneWireWriteByte(0x4E);		// Write Scratchpad.
		OneWireWriteByte(0x00);		// Th
		OneWireWriteByte(0x00);		// Tl
		OneWireWriteByte(0x1F);		// 9-bit resolutie
	}	

	if(ResetDS18B20())
	{
		OneWireWriteByte(0xCC);		// Skip ROM.
		OneWireWriteByte(0x44);		// Start conversion.
	}
}

int16_t ReadTemperatureDS18B20(void)
{
	int16_t temperatuur = -51;
	uint8_t dsData[9];
	
	if(ResetDS18B20())
	{
		OneWireWriteByte(0xCC);		// Skip ROM.
		OneWireWriteByte(0xBE);		// Read Scratchpad (8 bytes + CRC).
		
		dsData[0] = OneWireReadByte();
		dsData[1] = OneWireReadByte();
		
		// Als je slechts in de eerste twee bytes geïnteresseerd bent, lees er dan ook maar twee.		
//		dsData[2] = OneWireReadByte();
//		dsData[3] = OneWireReadByte();
//		dsData[4] = OneWireReadByte();
//		dsData[5] = OneWireReadByte();
//		dsData[6] = OneWireReadByte();
//		dsData[7] = OneWireReadByte();
//		dsData[8] = OneWireReadByte();
	}		
		
	// Omzetting naar gehele graden maken. Hier kan je ook nauwkeuriger werken, maar dan met kommagetallen...
	temperatuur = (int16_t)((dsData[1] << 8) + dsData[0]);
	temperatuur /=16;	
	
	// Volgende meting starten.
	if(ResetDS18B20())
	{
		OneWireWriteByte(0xCC);		// Skip ROM.
		OneWireWriteByte(0x44);		// Start conversion.
		
//		OneWireAsInput();					// Zolang conversie duurt, wordt bus laag gehouden. Controleer dat.
//		while(OneWireReadBit() == 0);		// Wachten op einde omzetting.	
	}
	
	return temperatuur;
}

//uint8_t ReadStatusDS18B20(void)
//{
//	uint8_t status = 0;
//	
//	if(ResetDS18B20())
//	{
//		OneWireWriteByte(0xAC);		// Start lezen van de STATUS byte.
//		status = OneWireReadByte();
//	}
//	
//	return status;
//}

void OneWireAsOutput(void)
{
	GPIOA->OTYPER = GPIOA->OTYPER | GPIO_OTYPER_OT_10;								// output open-drain
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER10) | GPIO_MODER_MODER10_0;		// output
}

void OneWireAsInput(void)
{
	GPIOA->MODER = GPIOA->MODER & ~GPIO_MODER_MODER10;								// input
}

void OneWireLow(void)
{
	GPIOA->ODR = GPIOA->ODR & ~GPIO_ODR_10;
}

void OneWireHigh(void)
{
	GPIOA->ODR = GPIOA->ODR | GPIO_ODR_10;
}

uint8_t OneWireReadBit(void)
{
	uint8_t bit = 0;
	
	OneWireLow();
	OneWireAsOutput();
	WaitForMicroseconds(2);							// Minimum 1 en maximum 15 µs.
	OneWireAsInput();
	WaitForMicroseconds(10);						// Master moet sample'n vóór 15µs. Kies hier 12µs.
	if((GPIOA->IDR & GPIO_IDR_10) == GPIO_IDR_10)
		bit = 1;
	WaitForMicroseconds(48);						// Aanvullen tot 60µs voor een time slot.	
	WaitForMicroseconds(2);							// 2µs recovery tijd laten.
	
	return bit;
}

void OneWireWriteBit(uint8_t bit)
{
	if(bit == 1)
	{
		// 1 schrijven naar de bus...
		OneWireLow();
		OneWireAsOutput();
		WaitForMicroseconds(7);			// Minimum 1 en maximum 15 µs.
		OneWireAsInput();
		WaitForMicroseconds(53);		// Aanvullen tot 60µs voor een time slot.
	}
	else
	{
		// 0 schrijven naar de bus...
		OneWireLow();
		OneWireAsOutput();
		WaitForMicroseconds(60);		// 60 µs laag houden.
		OneWireAsInput();
	}
	
	WaitForMicroseconds(2);				// 2µs recovery tijd laten.
}

void OneWireWriteByte(uint8_t data)
{
	uint8_t indexer = 0;
	
	for(indexer = 0; indexer < 8; indexer++)
	{
		if((data & 0x01) == 0x01)
			OneWireWriteBit(1);
		else
			OneWireWriteBit(0);
		
		data = data >> 1;
	}
}

uint8_t OneWireReadByte(void)
{
	uint8_t indexer = 0, data = 0;
	
	for(indexer = 0; indexer < 8; indexer++)
	{
		if(OneWireReadBit() == 1)
			data += 0x80;
		if(indexer < 7)
			data = data >> 1;
	}
	return data;
}

void WaitForMicroseconds(uint32_t microseconds)
{
	uint32_t temp;
	
	// Proefondervindelijk gekalibreerd met 1000 microseconden...
	for(temp = 0; temp < microseconds; temp++)
	{
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
	}
}
