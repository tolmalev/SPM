/*
 *   Software calibration of analog channels is available for DAQ boards:
 *
 *      NI PCI-6221
 *
 *   Calibration data, when available, are stored at install time into
 *       "/tmp/spm.calibration".
 *
 *   Two functions perform all conversions:
 *
 *       #include "calibration.h"
 *       float scale_adc_data(int raw_data)
 *
 *             converts the raw integer value read from ADC into the
 *             equivalent float value in Volt, 
 *
 *       int scale_dac_data (int channel, float data)
 *
 *             converts a float value into the corresponding raw integer
 *
 *       Calibration constants are unique for all ADC channels (so, no need
 *       to specify the channel in scale_adc_data() ), while every DAC
 *       channel has its own set (hence the correct DAC channel has to be
 *       passed to scale_dac_data () ).
 *
 *       scale_adc_data() and scale_dac_data() are fully transparent. When
 *       calibration data are not available, only int->float and float->int
 *       conversion is performed. 
 *
 *       A call to scale_initialize() is not required; it is useful only
 *       if late loading of calibration data may be of disadvantage.
 *
 */

#include "calibration.h"
#include "spm_types.h"
#include <stdio.h>
#include <string.h>

#define NAME "calibration.c"
#define HERE NAME, __FUNCTION__
#define CALIB_FILE "/tmp/spm.calibration"
#define DAC_MAX 6

static struct {
	int order;
	float c[4];
        char name[6];
} adc[1], dac[DAC_MAX];

static int started = 0;
static int n_adc = 0;
static int n_dac = 0;

/*
 *  look in /tmp for a file with calibration tables
 */
 
void scale_initialize() {

	FILE * datafile = NULL;
	char code[4]="";
        char name[6]="";
        int order;
        float c[4];
	int items;
        int lines=0;
        int j;

	started = 1;
	datafile = fopen (CALIB_FILE, "r");
	if (datafile == NULL) {
                printf ("%s:%s - %s \"%s\" %s\n", HERE,
                        "Calibration file",
                        CALIB_FILE,
                        "not found. No data conversion will be made");
                return;
        }

        while ( EOF != (items = fscanf (datafile, " %3s %5s %d %f %f %f %f ",
                                code, name, &order, c, c+1, c+2, c+3))) {
                lines++;
                if (items < 5 || order < 1 || order > 3) {
                        printf ("%s:%s %s %d in file %s.\n%s\n", HERE,
                                "bad format in line", lines, CALIB_FILE,
                                "All following lines ignored.");
                        fclose (datafile);
                        return;
                }
                if (strcmp(code,"ADC") == 0) {
                        if (n_adc == 0) {
                                adc->order = order;
                                for (j=0 ; j<3 ; j++) adc->c[j] = c[j];
                                strcpy (adc->name, name);
                                n_adc = 1;
                        } else {
                                printf ("%s:%s - %s %d. %s\n", HERE,
                                        "repeated ADC calibration at line",
                                        lines, "Ignored.");
                        }
                } else if (strcmp(code,"DAC") == 0) {
                        if (n_dac < DAC_MAX) {
                                dac[n_dac].order = order;
                                for (j=0 ; j<3 ; j++) dac[n_dac].c[j] = c[j];
                                strcpy (dac[n_dac].name, name);
                                n_dac++;
                        } else {
                                printf ("%s:%s - %s %d. %s\n", HERE,
                                        "excess DAC calibration data at line",
                                        lines, "Ignored.");
                        }
                } else {
                        printf ("%s:%s - %s %d %s \"%s\"%s\n", HERE,
                                "Bad format in line", lines,
                                "in file", CALIB_FILE,
                                "Calibration will be turned off.");
                        n_adc = n_dac = 0;
                        fclose (datafile);
                        return;
                }
        }
        printf ("%s:%s - loaded calibration table with %d ADC and %d DAC\n",
                HERE, n_adc, n_dac);
        fclose (datafile);
        return;
}

/*
 *  convert a raw data from ADC to a floating calibrated value
 */

float scale_adc_data(int raw) {

	int j;
        float scaled;

	if (!started) scale_initialize();

	if (n_adc) {
	        scaled = adc->c[adc->order];
	        for( j = adc->order-1 ; j >= 0 ; j--) {
	                scaled *= raw;
	                scaled += adc->c[j];
		}
		return scaled;
	} else {
		return (float) raw;
	}
}

/*
 *  convert a floating calibrated value to raw DAC data
 */
 
int scale_dac_data (int channel, float data) {

        if (!started) scale_initialize();

	if (n_dac > channel) {

	        return (int)
                        (data * (dac+channel)->c[1] + (dac+channel)->c[0]);
	}

	return (int) data;
}

#if CALIBRATION_TEST == 1

/*
 *  test ADC and DAC with scale calibration
 *
 *  use: ./test [DAC 0] [[DAC 1] [[DAC 2] [DAC 3]]]
 *              writes given DAC values (in Volt) to DAC 0 ... 4 after
 *              conversion to DAC internal format and reads and reports
 *              ADC values both in raw format and in Volt.
 *
 */

#include <stdio.h>
#include <string.h>

#include "calibration.h"

#define PARAMS_DIR "/sys/module/spm_fc/parameters/"

int main (int argc, char ** argv) {

        int j, ndac=0, dac[4];
        float value;
        int ivalue;
        char par_name[]=PARAMS_DIR"adc_0";
        char * par_code=par_name+strlen(PARAMS_DIR);
        FILE * param;

        for ( j=1 ; j<argc ; j++ ) {
                if (sscanf(argv[j], " %f ", &value) == 1) {
                         dac[ndac] = scale_dac_data (ndac, value);
                        if (++ndac == 4) break;
                } else break;
        }

        for ( j=0 ; j<ndac ; j++ ) {
                snprintf (par_code, 6, "dac_%d", j);
                param = fopen (par_name, "w");
                if (param) {
                        fprintf (param, "%d", dac[j]);
                        fclose (param);
                }
        }

        for ( j=0 ; j<8 ; j++ ) {
                snprintf (par_code, 6, "adc_%d", j);
                param = fopen (par_name, "r");
                if (param) {
                        fscanf (param, "%d", &ivalue);
                        printf ("ADC %d:  %6d  %f\n",
                                j, ivalue, scale_adc_data(ivalue));
                        fclose (param);
                }
        }
        return 0;
}

#endif
