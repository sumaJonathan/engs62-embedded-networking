
/*
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>		/* getchar,printf */
#include <stdlib.h>		/* strtod */
#include <stdbool.h>		/* type bool */
#include <unistd.h>		/* sleep */
#include <string.h>

#include "platform.h"
#include "xil_types.h"
#include "xparameters.h"	/* constants used by hardware */
#include "xttcps.h"			/* ttc timer function */
#include "xtmrctr.h"		/* axi timer counter functions */
#include "xadcps.h"			/* adc functions */
#include "xgpio.h"		/* axi gpio interface */

#include "adc.h"
#include "gic.h"
#include "io.h"
#include "led.h"
#include "servo.h"
#include "ttc.h"


/* Define constants */
#define CONFIGURE 0
#define PING 1
#define UPDATE 2
#define FREQ 1
#define ID 21

/* Define STATES */
#define TRAFFIC_ON 		0
#define YELLOW 			1
#define PEDESTRIAN 		2
#define TRAIN_COMING 	3
#define TRAIN_GONE 		4
#define MAINTENANCE 	5

/* Initialize iterators/Timers */
#define TRAFFIC_TMR 10
#define PEDESTRIAN_TMR 10  //@ 1/10TH seconds per interrupt
#define LIGHT_TMR 3

static int timercnt = 0; //timer counters
static int bluecount = 0;
static u8 t3min = 0;  //timer flags
static u8 t30sec = 0;
static u8 t20sec = 0;
static u8 t3sec = 0;
static float potvolt;
static double duty;

/* Crossing control flags */
static u8 keyflag = 0;
static u8 traincoming = 0;
static u8 btnpressed = 0;
static u8 pedcrossed = 0;


/* global variables for data grams and uart drivers */
static u8 ping_buffer[8];
static u8 update_buffer[132];
u8 mode = CONFIGURE;
static int ping_i = 0;
static int update_i = 0;

u8 state = TRAFFIC_ON;

typedef struct {
	int type; /* must be assigned to PING */
	int id; /* must be assigned to your id */
} ping_t;

typedef struct {
	int type; /* must be assigned to UPDATE */
	int id; /* must be assigned to your id */
	int value; /* must be assigned to some value */
} update_request_t;

typedef struct {
	int type;
	int id;
	int average;
	int values[30];
} update_response_t;

update_response_t* received_update;

/* handles UART initialization */
void uart_init();

/* handles state changing */
void change_state();

/* uart0 handler */
void uart_0_handler(void *CallBackRef, u32 Event, u32 EventData){
	u8 buffer0;
	XUartPs* uart0 = (XUartPs*) CallBackRef;
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(uart0, &buffer0, 1);
		if (mode == PING){
			if (ping_i < sizeof(ping_t) - 1){//buffer is not full
				ping_buffer[ping_i] = buffer0;
				ping_i ++;
			}
			else{
				printf("ping\n");
				ping_i = 0;
			}
		}
		else if (mode == UPDATE){
			if (update_i < sizeof(update_response_t) - 1){//buffer is not full
				update_buffer[update_i] = buffer0;
				update_i ++;
			}
			else{
				received_update = (update_response_t*) &update_buffer;
				float potv = (float)(received_update->values[ID])/100;
				duty = (double)((MAXDUTY-MINDUTY)*potv + MINDUTY);
				servo_set(duty);
				update_i = 0;
			}
		}
	else{
		XUartPs_Send(&uartp1, &buffer0, 1);
		}
	}
}


/* uart1 handler */
void uart_1_handler(void *CallBackRef, u32 Event, u32 EventData){
	u8 buffer;
	XUartPs* uart1 = (XUartPs*) CallBackRef;
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(uart1, &buffer, 1);
		XUartPs_Send(&uartp0, &buffer, 1);
		if (buffer == (u8)'\r'){
			buffer = (u8) '\n';
			XUartPs_Send(&uartp0, &buffer, 1);
		}
	}
}


/* handles button call-backs */
void main_btn_callback(u32 buttons) {
	if (buttons == 1 || buttons == 2){
		printf("Request crossing\n");
		btnpressed = 1;
		if(state == MAINTENANCE || state == TRAIN_COMING){
			led_set(4, LED_ON);
		}else{
			change_state();
		}
	}
}


/* handles switch call-backs */
void main_sw_callback(u32 sw_value){
	led_toggle(sw_value);		//for debugging purposes
	if (sw_value == 0) {			// Train coming switch
		if(state == TRAIN_COMING){
			printf("Train left\n");
			traincoming = 0;
			change_state();
		}else if (state == MAINTENANCE){
			if(!traincoming){
				servo_set(MAXDUTY);		//CLOSE GATE
				traincoming = 1;
			} else{
				servo_set(MINDUTY);		//OPEN GATE
				traincoming = 0;
			}
		} else{
			traincoming = 1;
			if(state == PEDESTRIAN){
				printf("Train arriving, closing gate!!!\n");
				servo_set(MAXDUTY);		//CLOSE GATE
			} else if(state == TRAFFIC_ON){
				printf("Train arriving, changing light to yellow!\n");
				led_set(Y_LED, LED_ON);
				change_state();
			}
		}
	} else if (sw_value == 1){		// Maintenance Key Switch
		if(state == MAINTENANCE){
			keyflag = 0;
			change_state();
		} else {
			keyflag = 1;
		}
	}
}

/*Handles ttc timer interrupts */
void main_ttc_callback(void){
	timercnt++;
	switch(state){
		case (TRAFFIC_ON):
			if(timercnt == TRAFFIC_TMR){
				t3min = 1;
				change_state();
			}
			break;
		case (YELLOW):
			if(timercnt == LIGHT_TMR){
				t3sec = 1;
				change_state();
			}
			break;
		case (PEDESTRIAN):
			if(timercnt == PEDESTRIAN_TMR){
				t30sec = 1;
				change_state();
			}
			break;
		case (TRAIN_GONE):
			if(timercnt == PEDESTRIAN_TMR){
				t20sec = 1;
				change_state();
			}
			break;
		case (MAINTENANCE):
			potvolt = adc_get_pot();
			duty = (double)((MAXDUTY-MINDUTY)*potvolt + MINDUTY);
			servo_set(duty);
			led_set(RED, LED_ON);

			if(timercnt % 2 == 0){  //say every other second
				bluecount++;    //increment the counter for blue light flash
				if(bluecount % 2 == 0){
					led_set(BLUE, LED_ON);
				}else{
					led_set(BLUE, LED_OFF);
				}
			}
			break;
	}
}

/* Main function */
int main()
{
    init_platform();
    gic_init(); /* initialize the gic (c.f. gic.h) */
	led_init();		/* Initialize LED module */
	io_btn_init(main_btn_callback);
	io_sw_init(main_sw_callback);

	ttc_init(FREQ, main_ttc_callback);
	ttc_start();	/* start ttc */
	servo_init();
	adc_init();
	uart_init();

    printf("Railway Crossing Traffic Control!\n");
    while(1){
    	sleep(1);
    }
    
    io_btn_close();
    io_sw_close();
    ttc_stop();
    ttc_close();

    gic_disconnect(XPAR_XUARTPS_1_INTR);
    gic_disconnect(XPAR_XUARTPS_0_INTR);
    gic_close();
    printf("DONE!!\n");
    cleanup_platform();
    return 0;
}

/* Handle UART initialization */
void uart_init(){
	XUartPs_Config* config = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	XUartPs_CfgInitialize(&uartp1, config, config->BaseAddress);
	XUartPs_SetInterruptMask(&uartp1, XUARTPS_IXR_RXOVR);
	XUartPs_SetFifoThreshold(&uartp1, 1);
	XUartPs_SetHandler(&uartp1, uart_1_handler, &uartp1);
	gic_connect(XPAR_XUARTPS_1_INTR, (Xil_ExceptionHandler)XUartPs_InterruptHandler,(void*) &uartp1);//connect the uart interrupt handler

	XUartPs_Config* config0 = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	XUartPs_CfgInitialize(&uartp0, config0, config0->BaseAddress);
	XUartPs_SetInterruptMask(&uartp0, XUARTPS_IXR_RXOVR);
	XUartPs_SetFifoThreshold(&uartp0, 1);
	XUartPs_SetHandler(&uartp0, uart_0_handler, &uartp0);
	XUartPs_SetBaudRate(&uartp0, 9600);
	gic_connect(XPAR_XUARTPS_0_INTR, (Xil_ExceptionHandler)XUartPs_InterruptHandler,(void*) &uartp0);

}

void change_state(void){
	switch(state){
	case (TRAFFIC_ON):
		led_set(GREEN, LED_ON); //turn green light on
		if((t3min && btnpressed) || traincoming || keyflag){
			state = YELLOW;
			timercnt = 0;
			t3min = 0;
		}
		break;

	case (YELLOW):
		led_set(Y_LED, LED_ON);	//turn yellow light on
			if(t3sec){
				if(btnpressed){
					state = PEDESTRIAN;
				} else if(traincoming){
					state = TRAIN_COMING;
				} else if(keyflag){
					state = MAINTENANCE;
				} else {
					state = TRAFFIC_ON;
				}
				timercnt = 0;
				t3sec = 0;
			}
		break;

	case (PEDESTRIAN):
		led_set(RED, LED_ON);	//turn red light on
			if(t30sec){
				state = YELLOW;
				t30sec = 0;
				timercnt = 0;
				pedcrossed = 1;
			}

			if(keyflag){
				state = MAINTENANCE;
				timercnt = 0;
				t30sec = 0;
			} else if(traincoming){
				state = TRAIN_COMING;
				timercnt = 0;
				t30sec = 0;
			}
		break;

	case (TRAIN_COMING):
		led_set(RED, LED_ON);	//turn red light on
		if(!traincoming){
			state = TRAIN_GONE;
			timercnt = 0;
		}

		break;

	case (TRAIN_GONE):
		if(t20sec){
			state = TRAFFIC_ON;
			servo_set(MINDUTY);
			timercnt = 0;
			t20sec = 0;
		}

		break;

	case (MAINTENANCE):
		if(!keyflag){
			if(btnpressed){
				led_set(4, LED_ON);
			}
			state = YELLOW;
		}
		break;

	}
}
