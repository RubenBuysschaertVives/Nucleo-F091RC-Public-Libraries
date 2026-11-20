#include "stm32f091xc.h"

#if !defined(DS18B20_DEFINED)
	#define DS18B20_DEFINED
	
	uint8_t ResetDS18B20(void);
	void OneWireAsOutput(void);
	void OneWireAsInput(void);
	void OneWireLow(void);
	void OneWireHigh(void);
	uint8_t OneWireReadBit(void);
	void OneWireWriteBit(uint8_t bit);
	void OneWireWriteByte(uint8_t data);
	uint8_t OneWireReadByte(void);
	void WaitForMicroseconds(uint32_t microseconds);
	void InitDS18B20(void);
	int16_t ReadTemperatureDS18B20(void);
	//uint8_t ReadStatusDS18B20(void);
#endif
