/*
 * io.h -- switch and button module interface
 *
 */

#include <stdio.h>			/* printf for errors */
#include <stdbool.h>
#include <xgpio.h>		  	/* axi gpio */
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */
#include "gic.h"


static void (*local_btn_callback)(u32 btn);
static void (*local_sw_callback)(u32 sw);

static XGpio btnport;	       /* btn GPIO port instance */
static XGpio swport;		   /* sw GPIO port instance */
static u32 prevState = 0;

#define CHANNEL1 0x1
#define BTNMASK 0XFF
#define SWMASK BTNMASK

/*
 * control is passed to this function when a button is pushed
 *
 * devicep -- ptr to the device that caused the interrupt
 */
static void btn_handler(void *devicep) {
	u32 btnstate = XGpio_DiscreteRead(&btnport, CHANNEL1);

	/* create interrupt only when pressing and not releasing */
	if (btnstate & BTNMASK){
		local_btn_callback(btnstate);
	}
	XGpio_InterruptClear(&btnport, XGPIO_IR_CH1_MASK);

}


static void sw_handler(void *devicep) {
	u32 switchState = XGpio_DiscreteRead(&swport, CHANNEL1);

	u32 swmask = switchState ^prevState;
	int i = 0;

	while (i < 4){
		if (swmask & (1 << i)){
			break;
		}
		i++;
	}

	local_sw_callback(i);	/*pass to callback function */

	XGpio_InterruptClear(&swport, XGPIO_IR_CH1_MASK); /* clear interrupt */

	prevState = switchState; /* update the current switch state to previous state */
}


/*
 * initialize the btns providing a callback
 */
void io_btn_init(void (*btn_callback)(u32 btn)){
	local_btn_callback = btn_callback; 	/*store button callback */
	XGpio_Initialize(&btnport, XPAR_AXI_GPIO_1_DEVICE_ID);
	XGpio_SetDataDirection(&btnport, CHANNEL1 , BTNMASK); //CHANGED THE 0x1 TO 0xFF configuring all GPIO pins to inputs

	XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK);  //Disable interrupts

	/* connect handler to gic */
	gic_connect(XPAR_FABRIC_GPIO_1_VEC_ID , (XExceptionHandler)btn_handler,  &btnport);

	XGpio_InterruptEnable(&btnport, XGPIO_IR_CH1_MASK); /* enable interrupts on channel (c.f. table 2.1) */

	XGpio_InterruptGlobalEnable(&btnport);/* enable interrupt to processor (c.f. table 2.1) */

}


/*
 * close the btns
 */
void io_btn_close(void){
	XGpio_InterruptDisable(&btnport, XGPIO_IR_CH1_MASK); /* enable interrupts on channel (c.f. table 2.1) */
	gic_disconnect(XPAR_FABRIC_GPIO_1_VEC_ID);	 /* disconnect the interrupts (c.f. gic.h) */
	local_btn_callback = NULL; //clear callback function
}


/*
 * initialize the switches providing a callback
 */
void io_sw_init(void (*sw_callback)(u32 sw)){
	local_sw_callback = sw_callback;

	XGpio_Initialize(&swport, XPAR_AXI_GPIO_2_DEVICE_ID);
	XGpio_SetDataDirection(&swport, CHANNEL1 , SWMASK);  /* set data direction to input */

	XGpio_InterruptDisable(&swport, XGPIO_IR_CH1_MASK);  /* Disable interrupt for connection to gic */

	/* connect handler to gic */
	gic_connect(XPAR_FABRIC_GPIO_2_VEC_ID , (XExceptionHandler)sw_handler,  &swport);

	XGpio_InterruptEnable(&swport, XGPIO_IR_CH1_MASK); /* enable interrupts on channel (c.f. table 2.1) */

	XGpio_InterruptGlobalEnable(&swport);/* enable interrupt to processor (c.f. table 2.1) */

}

/*
 * close the switches
 */
void io_sw_close(void){
	XGpio_InterruptDisable(&swport, XGPIO_IR_CH1_MASK); /* enable interrupts on channel (c.f. table 2.1) */
	gic_disconnect(XPAR_FABRIC_GPIO_2_VEC_ID);	 /* disconnect the interrupts (c.f. gic.h) */
	local_sw_callback = NULL; //clear callback function
}


