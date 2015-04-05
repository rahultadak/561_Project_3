#include "timers.h"
#include "MKL25Z4.h"
#include "ADC.h"
#include "LEDs.h"
#include "GPIO_defs.h"
#include "mma8451.h"

void Init_LPTMR(void) {
	SIM->SCGC5 |=  SIM_SCGC5_LPTMR_MASK;

	// Configure LPTMR
	// select 1 kHz LPO clock with prescale factor 0, dividing clock by 2
	// resulting in 500 Hz clock
	LPTMR0->PSR = LPTMR_PSR_PCS(1) | LPTMR_PSR_PRESCALE(0); 
	LPTMR0->CSR = LPTMR_CSR_TIE_MASK;
	LPTMR0->CMR = 50; // Generate interrupt every 50 clock ticks or 100 ms

	// Configure NVIC 
	NVIC_SetPriority(LPTimer_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(LPTimer_IRQn); 
	NVIC_EnableIRQ(LPTimer_IRQn);	

}

void Start_LPTMR(void) {
	LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;
}

void Stop_LPTMR(void) {
	LPTMR0->CSR &= ~LPTMR_CSR_TEN_MASK;
}

void LPTimer_IRQHandler(void) {
	static uint8_t n=LED_PERIOD;

	PTE->PSOR |= MASK(DEBUG_RUNNING_POS);
	NVIC_ClearPendingIRQ(LPTimer_IRQn);
	LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;

	read_full_xyz();
	convert_xyz_to_roll_pitch();
	
	if (n==0) 
	{
		Light_LEDs();
		n = LED_PERIOD;
	} 
	n--;
}

void Init_PIT(unsigned period) {
	// Enable clock to PIT module
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	// Enable module, freeze timers in debug mode
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;
	PIT->MCR |= PIT_MCR_FRZ_MASK;
	
	// Initialize PIT0 to count down from argument 
	PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(period);

	// No chaining
	PIT->CHANNEL[0].TCTRL &= PIT_TCTRL_CHN_MASK;
	
	// Generate interrupts
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;

	/* Enable Interrupts */
	NVIC_SetPriority(PIT_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(PIT_IRQn); 
	NVIC_EnableIRQ(PIT_IRQn);	
}


void Start_PIT(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}

void Stop_PIT(void) {
// Enable counter
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}


void PIT_IRQHandler() {
	//float v_rail;
	static uint8_t n=0;
	static uint8_t n_clk=2;
	static uint8_t divider = 1;
	
	//clear pending IRQ
	NVIC_ClearPendingIRQ(PIT_IRQn);
	// check to see which channel triggered interrupt 
	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 0
		PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
		
		// Do ISR work
		if (n) {
			//v_rail = Measure_VRef();
			//Light_LEDs(v_rail);
		} else {
			Control_RGB_LEDs(0,0,0);
		}
		n = 1-n;

#if 1		
		if (--n_clk == 0) {
			// change clock frequency
			n_clk = 2;
			SIM->CLKDIV1 = (SIM->CLKDIV1 & ~SIM_CLKDIV1_OUTDIV1_MASK) | 
					SIM_CLKDIV1_OUTDIV1(divider);
			if (++divider == 16) {
				divider = 1;
			}
		}
#endif 
/*
		if (v_rail < 1.8) {
			Control_RGB_LEDs(1, 0, 0);
		}		
*/
	} else if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		// clear status flag for timer channel 1
		PIT->CHANNEL[1].TFLG &= PIT_TFLG_TIF_MASK;
	} 
}

void Init_TPM(uint32_t period_ms)
{
	//turn on clock to TPM 
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	//set clock source for tpm to be TPM Source 3 which is MCGIR (32 kHz)
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(3);

	// disable TPM
	TPM0->SC = 0;

	//set TPM to count up and divide by 32 with prescaler and use clock mode
	// will count at 1 kHz
	TPM0->SC = (TPM_SC_PS(5));
	
	//load the counter and mod. Note: 16-bit counter, not 32-bit
	TPM0->MOD = TPM_MOD_MOD(period_ms);

	
	TPM0->SC |= TPM_SC_TOIE_MASK;

	// Configure NVIC 
	NVIC_SetPriority(TPM0_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(TPM0_IRQn); 
	NVIC_EnableIRQ(TPM0_IRQn);	
}

void TPM0_IRQHandler() {
	//float v_rail;
	static uint8_t n=0;
	static uint8_t n_clk=2;
	static uint8_t divider = 1;
	
	//clear pending IRQ
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	
	TPM0->SC |= TPM_SC_TOF_MASK; 

	// Do ISR work
	if (n) {
		//v_rail = Measure_VRef();
		//Light_LEDs(v_rail);
	} else {
		Control_RGB_LEDs(0,0,0);
	}
	n = 1-n;
#if 1		
	if (--n_clk == 0) {
		// change clock frequency
		n_clk = 2;
		SIM->CLKDIV1 = (SIM->CLKDIV1 & ~SIM_CLKDIV1_OUTDIV1_MASK) | 
				SIM_CLKDIV1_OUTDIV1(divider);
		if (++divider == 16) {
			divider = 1;
		}
	}
#endif 
}

void Start_TPM(void) {
// Enable counter
	TPM0->SC |= TPM_SC_CMOD(1);
}


// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
