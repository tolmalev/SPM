/*
 *
 * Copyright (C) 2008 Giovanni Aloisi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (www.fsf.org).
 *
 * Address of the author:
 * Department of Chemistry, University of Florence
 * Via Della Lastruccia 3
 * Sesto F.no (Firenze) I 50019 - Italy
 * e-mail: aloisi@unifi.it
 */

/*
 *   Simple routine to test ADC and DAC channels
 *   install with: ./install ufc=testchans
 *   use gamp to read the ADCs and to write to DACs; modify as required
 */


/* input signals */

static int adc_0=0, adc_1=0, adc_2=0, adc_3=0;
static int adc_4=0, adc_5=0, adc_6=0, adc_7=0;

module_param (adc_0, int, S_IRUGO);
module_param (adc_1, int, S_IRUGO);
module_param (adc_2, int, S_IRUGO);
module_param (adc_3, int, S_IRUGO);
module_param (adc_4, int, S_IRUGO);
module_param (adc_5, int, S_IRUGO);
module_param (adc_6, int, S_IRUGO);
module_param (adc_7, int, S_IRUGO);

/* output signals */

static int dac_0=0, dac_1=0, dac_2=0;
static int dac_3=0, dac_4=0, dac_5=0;

module_param (dac_0, int, S_IRUGO | S_IWUSR);
module_param (dac_1, int, S_IRUGO | S_IWUSR);
module_param (dac_2, int, S_IRUGO | S_IWUSR);
module_param (dac_3, int, S_IRUGO | S_IWUSR);
module_param (dac_4, int, S_IRUGO | S_IWUSR);
module_param (dac_5, int, S_IRUGO | S_IWUSR);

static int shift=0;
module_param (shift, int, S_IRUGO | S_IWUSR);


/* diagnostics:  interrupts */
static int counter = 0;
module_param (counter, int, S_IRUGO); /* number of interrupts serviced; */
static int countfails = 0;
module_param (countfails, int, S_IRUGO); /* number of buffer errors*/


void feedback_code (struct irq_parameters * irq) {

	counter += 1;
	if (irq->data->r_adc != irq->sample_adc * irq->samples_per_point)
                countfails += 1;        /* bad filled data buffer */

/*****    INPUT VALUES *****/

	if (irq->sample_adc > 0) adc_0 = irq->adc[0];
	if (irq->sample_adc > 1) adc_1 = irq->adc[1];
	if (irq->sample_adc > 2) adc_2 = irq->adc[2];
	if (irq->sample_adc > 3) adc_3 = irq->adc[3];
	if (irq->sample_adc > 4) adc_4 = irq->adc[4];
	if (irq->sample_adc > 5) adc_5 = irq->adc[5];
	if (irq->sample_adc > 6) adc_6 = irq->adc[6];
	if (irq->sample_adc > 7) adc_7 = irq->adc[7];
	



/*****  OUT VALUES *****/

	if (irq->sample_dac > 0) irq->dac[0] = dac_0;
	if (irq->sample_dac > 1) irq->dac[1] = dac_1;
	if (irq->sample_dac > 2) irq->dac[2] = dac_2;
	if (irq->sample_dac > 3) irq->dac[3] = dac_3;
	if (irq->sample_dac > 4) irq->dac[4] = dac_4;
	if (irq->sample_dac > 5) irq->dac[5] = dac_5;

	irq->data->byte[0] = 0;
	irq->data->byte[1] = 0;
	
	dac_0 ^= shift;
}
