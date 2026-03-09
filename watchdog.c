#include "stm32f091xc.h"
#include "watchdog.h"

void InitWatchdog(void)
{
	IWDG->KR = 0x0000CCCC;		// Enable de watchdog.
	IWDG->KR = 0x00005555;		// Schrijftoegang tot registers van watchdog verschaffen.
	IWDG->PR = IWDG_PR_PR;		// divider /256	=> bronklok is 40kHz => 6,4ms per stap.
	IWDG->RLR = 1999;			// 2000 x 6,4ms = 12,8s...  (telt af naar 0)
	while(IWDG->SR != 0);		// Wacht tot bovenstaande registers geüpdatet zijn.
	IWDG->KR = 0x0000AAAA;		// Reset de watchdog en start hem.
}

void ResetWatchdog(void)
{
	IWDG->KR = 0x0000AAAA;		// Reset de watchdog.
}
