/*
 * led.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>							/* printf(), getchar() */
#include "xil_types.h"					/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include <xgpio.h>							/* Xilinx GPIO functions */
#include "xparameters.h"				/* constants used by the hardware */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "led.h"
#include <xgpiops.h>


#define OUTPUT 0x0							/* setting GPIO direction to output */
#define CHANNEL1 1							/* channel 1 of the GPIO port */

static u32 ledstate = 0x0;
static u32 rgbstate = 0x0;
static XGpioPs gpio;

static XGpio port2;
static XGpio port;

/*
 *
 * Initialize the led module
 */
void led_init(void){
									/* GPIO port connected to the leds */
    init_platform();							/* initialize the hardware platform */

    XGpio_Initialize(&port, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
    XGpio_SetDataDirection(&port, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */

    XGpio_Initialize(&port2, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_ */
    XGpio_SetDataDirection(&port2, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */

    XGpioPs_Config *config;

    config = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID );
    XGpioPs_CfgInitialize(&gpio, config, config->BaseAddr);

    XGpioPs_SetDirectionPin(&gpio, 7, 1);
    XGpioPs_SetOutputEnablePin(&gpio, 7, 1);

}

/*
 * Set <led> to one of {LED_ON,LED_OFF,...}
 *
 * <led> is either ALL or a number >= 0
 * Does nothing if <led> is invalid
 */
void led_set(u32 led, bool tostate){								/* GPIO port connected to the leds */
    if (led == ALL){
        ledstate = tostate ? 0xF : 0x0;
    } else if (led >= 0 && led <= 3){
        if (tostate){
            ledstate |= 1 << led;
        } else {
            ledstate &= ~(1 << led);
        }
    }
    else if (led == 4){
    	if (tostate){
			XGpioPs_WritePin(&gpio, 7, 1);
		} else {
			XGpioPs_WritePin(&gpio, 7, 0);
		}
    }
    else if (led == 5){		/* Red */
    	if (tostate){
			rgbstate = 2;
		} else {
			rgbstate = 0;
		}XGpio_DiscreteWrite(&port2, CHANNEL1, rgbstate);
    }
    else if (led == 6){ 	/* Green */
    	if (tostate){
			rgbstate = 1;
		} else {
			rgbstate = 0;
		}XGpio_DiscreteWrite(&port2, CHANNEL1, rgbstate);
    }
    else if (led == 7){		/* Blue */
    	if (tostate){
			rgbstate = 4;
		} else {
			rgbstate = 0;
		}XGpio_DiscreteWrite(&port2, CHANNEL1, rgbstate);
    }
    else if (led == 8){		/* Yellow */
    	if (tostate){
			rgbstate = 6;
		} else {
			rgbstate = 0;
		}XGpio_DiscreteWrite(&port2, CHANNEL1, rgbstate);
    }
    XGpio_DiscreteWrite(&port, CHANNEL1, ledstate);
}

/*
 * Get the status of <led>
 *
 * <led> is a number >= 0
 * returns {LED_ON,LED_OFF,...}; LED_OFF if <led> is invalid
 */
bool led_get(u32 led){
    XGpio port;									/* GPIO port connected to the leds */
    XGpio_Initialize(&port, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
    XGpio_SetDataDirection(&port, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */
    u32 currState = ledstate;
    if (led >= 0 && led <= 3){
        return (currState >> led) & 0x1;
    }
    else if (led == 4){
    	return XGpioPs_ReadPin(&gpio, 7);
    }
    return LED_OFF;
}

/*
 * Toggle <led>
 *
 * <led> is a number >= 0
 * Does nothing if <led> is invalid
 */
void led_toggle(u32 led){
    XGpio port;									/* GPIO port connected to the leds */
    XGpio_Initialize(&port, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
    XGpio_SetDataDirection(&port, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */
    if (led >= 0 && led <= 3){
        ledstate ^= 1 << led;
        XGpio_DiscreteWrite(&port, CHANNEL1, ledstate);
    }
}
