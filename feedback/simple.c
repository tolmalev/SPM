/*
 *
 * Copyright (C) 2008-2009 Giovanni Aloisi
 *
 * This program is free software; you can use, modify and redistribute
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (www.fsf.org).
 *
 * Address of the author:
 * Department of Chemistry, University of Florence
 * Via Della Lastruccia 3
 * Sesto F.no (Firenze) I 50019 - Italy
 * e-mail: aloisi_at_unifi.it
 */

/*
 *   Simple example of feedback routine: the system is assumed to have a
 *   master board with:

 *      ADC channel 0 connected to the probe pre-amplifier;
 *      DAC channel 0 connected to the Z piezo amplifier (not directedly!,
 *          use a buffer with protected output); 
 *      DAC channel 1 connected to the engage motor: you have to trim the
 *          voltage (again, you need a buffer);
 *      DAC channel 3 and 4 (if available) are reserved to piezo's for
 *          X and Y axis scan
 *
 *   You are supposed to trim scanning parameters through 'user/gamp' and
 *   to use 'user/frame' to scan the surface.
 *   Voltange range for all ADC/DAC input/output is from -10 to +10 V, with
 *   16 bit (64 k-channels); on board calibration is not used, hence 1 volt
 *   is only approximately 3200 channels.
 *
 *   To engage set 'vareng' to 1 with 'user/gamp'. Once engaged 'vareng'
 *   will go back to zero.
 *   Look at 'ivout0' for the height of the probe and 'ivin0' for the probe
 *   signal.
 *
 *   You may try:
 *       user/frame -p 500 -l 2 -L 2
 *   or:
 *       user/frame -p 500 -l 500 -F
 *
 *   Once done, it is up to you to lift the probe.
 */

/* limits output voltage to +-10V */

#define UPLIMIT 32000
#define LOWLIMIT -32000
#define PIDDIV 100000    /* used to avoid overflow in multiplications */

/*
 *   Parameters that will be accessible through the /sys filesystem.
 */

static int pidset = -320;
module_param (pidset, int, S_IRUGO | S_IWUSR); /* Setpoint for channel 0 */

static int pidkp = 2;
module_param (pidkp, int, S_IRUGO | S_IWUSR);  /* Proportional constant */

static int pidki = 2000;
module_param (pidki, int, S_IRUGO | S_IWUSR);  /* Integral constant */

static int pidpol = -1;
module_param (pidpol, int, S_IRUGO | S_IWUSR); /* error polarity */

/* Engage variables */

static int vareng = 0;	     /* set to one to activates the motor for engage */
module_param (vareng, int, S_IRUGO | S_IWUSR);

static int voleng = 3200; /* when Z is voleng from extremes probe is engaged */
module_param (voleng, int, S_IRUGO | S_IWUSR);

/* signals monitor */

static int ivin0 = 0; /* input signal */
module_param (ivin0, int, S_IRUGO);

static int ivout0=0, ivout1=0, ivout2=0, ivout3=0; /* output signals */
module_param (ivout0, int, S_IRUGO);
module_param (ivout1, int, S_IRUGO | S_IWUSR);
module_param (ivout2, int, S_IRUGO);
module_param (ivout3, int, S_IRUGO);

/* ramp parameters */

static int framex0 = 0, framey0 = 0; /* frame origin */
module_param (framex0, int, S_IRUGO | S_IWUSR);
module_param (framey0, int, S_IRUGO | S_IWUSR);

/* increment per step: 3200 -> 1 mV */
static int dvxpp1000 = 3200, dvypp1000 = 3200; 
module_param (dvxpp1000, int, S_IRUGO | S_IWUSR);
module_param (dvypp1000, int, S_IRUGO | S_IWUSR);

/* diagnostics:  interrupts */
static int counter = 0;
module_param (counter, int, S_IRUGO); /* number of interrupts serviced; */
static int countfails = 0;
module_param (countfails, int, S_IRUGO); /* number of buffer errors*/

void feedback_code (struct irq_parameters * irq) {

	static int  error, errold = 0;
	int errcor = 0; 		/*  correction to apply*/
	int pramp, pline;		/*  points in line, lines in frame */
	int xramp, yramp;		/*  x,y positions */
	counter += 1;

/*****    INPUT VALUES *****/

	ivin0 = irq->adc[0];
	if (irq->data->r_adc == irq->sample_adc * irq->samples_per_point) {
/* otherwise skip */

/* compute the Z feedback value */

		error  = (pidset - ivin0) * pidpol;
		errcor = error * (pidkp + pidki) + errold * (pidki - pidkp);
		errold = error;
		ivout0 = ivout0 + errcor / PIDDIV;
		if(ivout0 > UPLIMIT ) ivout0 = UPLIMIT;
		if(ivout0 < LOWLIMIT) ivout0 = LOWLIMIT;

/*****    ENGAGE? *****/

		if (ivout0 < (UPLIMIT-voleng) && ivout0 > (LOWLIMIT+voleng))
			vareng = 0;   /* stop engagement, if active */
		ivout1 = 0;           /* motor stopped by default */
		if(vareng) ivout1 = 16000;          /* motor on */
	} else {       /* fifo error: record it and leave ivout0 unchanged */
		countfails += 1;
	}

	/*****  do the above always,
		but on irq->action go also for XY ramps calculation *****/

	switch (irq->action) {

	case Go_Process:

		pramp = irq->points_per_line - irq->points;
		pline = irq->lines_per_frame - irq->lines;
  

		if ((pline % 2) == 0) {	 /*  even line, regular */

			xramp = framex0 + (dvxpp1000 * pramp) / 1000;
			yramp = framey0 + (dvypp1000 * pline) / 1000;
		} else { 		  /*  odd line, reversed */

			xramp = framex0 + (dvxpp1000 * irq->points) / 1000;
			yramp = framey0 + (dvypp1000 * pline) / 1000;
 
		}
		ivout2 = xramp;
		ivout3 = yramp;

		break;

	case Idle:       /*  nothing happens  */
		break;
	}

/*****  OUT VALUES *****/

	irq->dac[0] = ivout0;
	irq->dac[1] = ivout1;
	irq->dac[2] = ivout2;
	irq->dac[3] = ivout3;
	irq->dac[4] = 0;
	irq->dac[5] = 0;
	irq->data->byte[0] = 0;
	irq->data->byte[1] = 0;
}
