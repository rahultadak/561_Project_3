/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include "gpio_defs.h"
#include "LEDs.h"
#include "timers.h"
#include "delay.h"
#include "ADC.h"
#include "config.h"
#include "mma8451.h"
#include "i2c.h"

void Init_Debug_Signals(void) {
	// Enable clock to ports E
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	// Make 3 pins GPIO
	PORTE->PCR[DEBUG_RUNNING_POS] &= ~PORT_PCR_MUX_MASK;          
	PORTE->PCR[DEBUG_RUNNING_POS] |= PORT_PCR_MUX(1);          
	
	// Set ports to outputs
	PTE->PDDR |= MASK(DEBUG_RUNNING_POS);
	
	PTE->PSOR |= MASK(DEBUG_RUNNING_POS);
}	
	

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
uint8_t smc_pmstat;
int main (void) {
		uint16_t res=0;
		
		//Using System Integration Module Clock divider to reduce system clock
		SIM->CLKDIV1 = (0x1 << SIM_CLKDIV1_OUTDIV1_SHIFT) | (0x5 << SIM_CLKDIV1_OUTDIV4_SHIFT) ;

		//Selecting Fast IRC
		MCG->C2 |= MCG_C2_IRCS_MASK;
		
		//Enabling Internal Reference Clock
		MCG->C1 |= MCG_C1_CLKS(2);
		MCG->C1 &= ~MCG_C1_IREFS_MASK;
		//Disabling PLLs
		MCG->C6 &= ~MCG_C6_PLLS_MASK;

		//Check if clock change has completed
		while(!(MCG->S & MCG_S_IREFST_MASK >> MCG_S_IREFST_SHIFT))
		{	;}
		//Enabling Low Power Mode
		MCG->C2 |= MCG_C2_LP_MASK;	

		// Allow low leakage stop mode
		SMC->PMPROT = SMC_PMPROT_ALLS_MASK | SMC_PMPROT_AVLLS_MASK | SMC_PMPROT_AVLP_MASK; // 
		// Enable low-leakage stop mode and regular run mode
		SMC->PMCTRL = SMC_PMCTRL_STOPM(3) | SMC_PMCTRL_RUNM(2);
		SMC->STOPCTRL = SMC_STOPCTRL_PSTOPO(0) | SMC_STOPCTRL_VLLSM(3);

		// Enable LLWU
		// allow LPTMR0 to wake LLWU
		LLWU->ME |= LLWU_ME_WUME0_MASK;
	
	// Enable stop mode (deep sleep)
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	
		Init_RGB_LEDs();
		i2c_init();																/* init i2c	*/
	
		res = init_mma();													/* init mma peripheral */
		if (res == 0) {
		// Signal error condition
			Control_RGB_LEDs(1, 0, 0);
			while (1)
				;
		}		
	
		Init_LPTMR();
		Start_LPTMR();
	
		__enable_irq();
	
		// work is in interrupt
		while (1) {
#if USE_SLEEP_MODES
		__wfi() ; // then go to sleep	
#endif
		}
}

// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
