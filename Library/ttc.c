/*
 * ttc.c
 *
 * NOTE: The TTC hardware must be enabled (Timer 0 on the processing system) before it can be used!!
 *
 */


#include "ttc.h"		/* include header file*/
#include "gic.h"
#include "led.h"


static void (*local_ttc_callback)(void);
static XTtcPs ttcPort; /* define ttc instance */


static void ttc_handler(void* devicePtr){
	local_ttc_callback();
	XTtcPs_ClearInterruptStatus(&ttcPort, XTTCPS_IXR_INTERVAL_MASK);
}




/*
 * ttc_init -- initialize the ttc freqency and callback
 */
void ttc_init(u32 freq, void (*ttc_callback)(void)){

	XTtcPs_Config* ttcConfig;
	u8 prescaler;
	XInterval interval;

	local_ttc_callback = ttc_callback;
	ttcConfig = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);

	XTtcPs_CfgInitialize(&ttcPort, ttcConfig, ttcConfig->BaseAddress);

	XTtcPs_DisableInterrupts(&ttcPort, XTTCPS_IXR_INTERVAL_MASK);

	/*connect interrupt handler to gic */
	gic_connect(XPAR_XTTCPS_0_INTR, (XExceptionHandler)ttc_handler, &ttcPort);

	/*set up ttc prescaler and mode*/
	XTtcPs_CalcIntervalFromFreq(&ttcPort, freq, &interval, &prescaler);
	XTtcPs_SetPrescaler(&ttcPort, prescaler);
	XTtcPs_SetInterval(&ttcPort, interval);
	XTtcPs_SetOptions(&ttcPort,XTTCPS_OPTION_INTERVAL_MODE);

	/* Enable interrupts at ttc level */
	XTtcPs_EnableInterrupts(&ttcPort, XTTCPS_IXR_INTERVAL_MASK);

}

/*
 * ttc_start -- start the ttc
 */
void ttc_start(void){
	XTtcPs_Start(&ttcPort);

}

/*
 * ttc_stop -- stop the ttc
 */
void ttc_stop(void){
	XTtcPs_Stop(&ttcPort);
	led_set(4,LED_OFF);
}

/*
 * ttc_close -- close down the ttc
 */
void ttc_close(void){
	XTtcPs_DisableInterrupts(&ttcPort, XTTCPS_IXR_INTERVAL_MASK);

	gic_disconnect(XPAR_XTTCPS_0_INTR);
}


