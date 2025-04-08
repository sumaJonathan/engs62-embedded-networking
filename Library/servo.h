/*
 * servo.h
 */
#pragma once

#include <stdio.h>
#include "xtmrctr.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define OPTIONS (XTC_PWM_ENABLE_OPTION | XTC_EXT_COMPARE_OPTION | XTC_DOWN_COUNT_OPTION)
#define CLOCK_FREQ 50000000 /*internal clock frequency */
#define PERIOD	20/1000	/*period of pwm waveform */
#define MAXDUTY 0.1019 //0.125
#define MINDUTY 0.0556 //0325

/*
 * Initialize the servo, setting the duty cycle to 7.5%
 */
void servo_init(void);

/*
 * Set the dutycycle of the servo
 */
void servo_set(double dutycycle);



