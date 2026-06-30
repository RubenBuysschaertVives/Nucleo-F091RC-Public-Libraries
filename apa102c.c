#include "stm32f091xc.h"
#include "apa102c.h"
#include "spi1.h"

APA102C leds[NUMBER_OF_APA102C_LEDS];

// Verstuur de data over SPI1. Let op: zorg dat SPI1 reeds geïnitialiseerd is.
void UpdateAPA102CLeds(APA102C led[])
{
	uint8_t index = 0;
	
	ByteToSpi1(0);															// Start frame.
	ByteToSpi1(0);
	ByteToSpi1(0);
	ByteToSpi1(0);
	
	for(index = 0; index < NUMBER_OF_APA102C_LEDS; index++)
	{	
		ByteToSpi1(led[index].brightness | 0b11100000);		// 5-bit intensiteit.
		ByteToSpi1(led[index].blue);											// Blauw.
		ByteToSpi1(led[index].green);											// Groen.
		ByteToSpi1(led[index].red);												// Rood.
	}
	
	ByteToSpi1(255);														// End frame.
	ByteToSpi1(255);
	ByteToSpi1(255);
	ByteToSpi1(255);
}

void ClearAPA102CLeds(APA102C leds[])
{
	for(uint8_t i = 0; i < NUMBER_OF_APA102C_LEDS; i++)
	{
		leds[i].brightness = DEFAULT_LED_BRIGHTNESS;
		leds[i].red = 0;
		leds[i].green = 0;
		leds[i].blue = 0;		
	}
	
	UpdateAPA102CLeds(leds);
}

void SetAPA102CLed(APA102C leds[], uint8_t ledNumber, uint8_t red, uint8_t green, uint8_t blue)
{
	ClearAPA102CLeds(leds);
	
	if(ledNumber < NUMBER_OF_APA102C_LEDS)
	{
		leds[ledNumber].brightness = DEFAULT_LED_BRIGHTNESS;
		leds[ledNumber].red = red;
		leds[ledNumber].green = green;
		leds[ledNumber].blue = blue;
	}
	
	UpdateAPA102CLeds(leds);
}