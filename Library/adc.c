/*
 * adc.c -- ADC module interface function definitions
 */

#include "adc.h"

#define CHANNELS XADCPS_SEQ_CH_TEMP | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_AUX14


static XAdcPs adc_port;		/* adc port for temperature */

/*
 * initialize the adc module
 */
void adc_init(void){
	XAdcPs_Config *adcCfg = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID);

	XAdcPs_CfgInitialize(&adc_port, adcCfg, adcCfg->BaseAddress); /*initialize adc */

	if (XAdcPs_SelfTest(&adc_port)!= XST_SUCCESS){	/* test the adc port */
		printf("ADC Test Failed!\n");
	}
	XAdcPs_SetSequencerMode(&adc_port, XADCPS_SEQ_MODE_SAFE);	/* set sequencer to safe mode first*/
	XAdcPs_SetAlarmEnables(&adc_port, 0);		/* enable alarms */
	XAdcPs_SetSeqChEnables(&adc_port, CHANNELS);
	XAdcPs_SetSequencerMode(&adc_port, XADCPS_SEQ_MODE_CONTINPASS);	/* set sequencer to continuous pass mode */

}


/*
 * get the internal temperature in degree's centigrade
 */
float adc_get_temp(void){
	u32 adctemp = XAdcPs_GetAdcData(&adc_port, XADCPS_CH_TEMP); /* get data from the temp ch */
	return XAdcPs_RawToTemperature(adctemp);

}

/*
 * get the internal vcc voltage (should be ~1.0v)
 */
float adc_get_vccint(void){
	u32 adcVcc = XAdcPs_GetAdcData(&adc_port, XADCPS_CH_VCCINT);
	return XAdcPs_RawToVoltage(adcVcc);

}

/*
 * get the **corrected** potentiometer voltage (should be between 0 and 1v)
 */
float adc_get_pot(void){
	u32 potadc = XAdcPs_GetAdcData(&adc_port, XADCPS_CH_AUX_MAX-1);
	float potvcc = XAdcPs_RawToVoltage(potadc)/2.97; /* voltage range 0-1V */
	return potvcc;

}

