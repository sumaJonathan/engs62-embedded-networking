#include "servo.h"
#define OPTIONS (XTC_PWM_ENABLE_OPTION | XTC_EXT_COMPARE_OPTION | XTC_DOWN_COUNT_OPTION)

/* define timer stuff */
static XTmrCtr timer;
void servo_init(void){
	XTmrCtr_Initialize(&timer, XPAR_XTTCPS_0_DEVICE_ID);	/*initialized AXI timer */

	XTmrCtr_SetResetValue(&timer, 0, CLOCK_FREQ * PERIOD);	/*counter for period */
	XTmrCtr_SetResetValue(&timer, 1, CLOCK_FREQ * PERIOD * 0.075);	/*counter for high time */


	XTmrCtr_SetOptions(&timer, 0, OPTIONS);	/*enable pwm option for both tmr_ctrs */
	XTmrCtr_SetOptions(&timer, 1, OPTIONS);

	XTmrCtr_Start(&timer, 0);	/*Start tmr_ctrs */
	XTmrCtr_Start(&timer, 1);

}

void servo_set(double dutycycle){
//	u32 rst = CLOCK_FREQ * PERIOD* dutycycle;
	double duty;
	if (dutycycle > MAXDUTY){
		duty = MAXDUTY;
	}
	else if (dutycycle < MINDUTY){
		duty = MINDUTY;
	}
	else{
		duty = dutycycle;
	}
	XTmrCtr_SetResetValue(&timer, 1, CLOCK_FREQ * PERIOD * duty);
}
