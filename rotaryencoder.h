#include "stm32f091xc.h"
#include "stdbool.h"

#if !defined(ROTARY_ENCODER_DEFINED)
	#define ROTARY_ENCODER_DEFINED
	
	// Wil je de drukknnop van de rotary encoder gebruiken? Heb je SB21 verwijderd?
	//#define ROTARY_ENCODER_USE_SWITCH
	
	// Wil je met interrupts werken? Haal dan onderstaande uit commentaar.
	// Merk op dat dit de beste optie is!
	#define ROTARY_ENCODER_USE_INTERRUPTS
	
	// Functieprototypes (afhankelijk van de defines in de header file).
	void InitRotaryEncoder(void);
	#if defined(ROTARY_ENCODER_USE_SWITCH)
		bool RotaryEncoderSwitchActive(void);		
	#endif
	#if !defined(ROTARY_ENCODER_USE_INTERRUPTS)
		bool RotaryEncoderAActive(void);
	#endif	
	bool RotaryEncoderBActive(void);
#endif