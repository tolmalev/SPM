/*
 *
 *  niconf.c
 *
 *          configure and test National Instruments DAQ boards
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "spm_types.h"
#include "nilib.h"
#include "register_macro.h"
#include "spm_dev_data.h"
#include "nilib_dma.h"
#include "Mite_registers.h"
#include "tMite.h"

#define NAM "niconf"
#define NUMBEROFSAMPLES 20
#define NUMBEROFCHANNELS 8
#define SAMPLEPERIODDIVISOR 1000000

#define INBUFSIZ 4096

#define PHYSICAL_ADDRESS "/sys/module/nibac/parameters/dma_address"
#define DMA_USE "/sys/module/nibac/parameters/dma_use"
#define SAMPLES_PP "/sys/module/spm_dev/parameters/samples_per_point"
#define CHANNELS "/sys/module/spm_dev/parameters/sample_adc"

/* timebase/sample rate => 20 MHz / 10 kHz */

#define display \
"Usage: adc [OPTION]\n"\
"\n"\
"working parameters\n\n"\
" -c   <int> number of channels [8]\n"\
" -d   <int> 20MHz timebase divisor [2000000]\n"\
" -r   <int> conversion rate [80]\n"\
" -s   <int> number of samples [20]\n"\
" -p   <int> repetition per sample [1]\n"\
" -e   ignore the -s value and go on endless\n\n"\
" -x   <int> +/-V adc range: 0 ->10V; 1 -> 5V; 4 -> 1V; 5 -> 0.2V [0]\n\n"\
" -F   setup the board for poll read from user space\n"\
" -I   setup the board for kernel space interrupt driven read from fifo\n"\
" -M   with -I setup: read through a DMA channel\n\n"\
" -R   set -M option according to dma_use in module\n"\
" -t   arm and start the board\n"\
" -i   with -F setup: read data\n"\
" -D   loop ADC's data to DAC's\n"\
" -T   report extended conversion times\n"\
" -o   overwrite the same output line\n\n"\
" -b   <int> write byte to digital lines (master board)\n"\
" -B   <int> write byte to digital lines (slave board)\n\n"\
" -v   debug\n"\
" -h   this help\n"

float msec(struct timeval * after, struct timeval * before) {
	return ((after->tv_sec - before->tv_sec) * 1000. +
		(after->tv_usec - before->tv_usec)/1000.);
}

long int usec(struct timeval * after, struct timeval * before) {
	return ((after->tv_sec - before->tv_sec) * 1000000 +
		(after->tv_usec - before->tv_usec));
}

int main (int argc, char ** argv) {

	u8  ** board[2];

	u32 numberOfSamples     = NUMBEROFSAMPLES;
	u32 numberOfChannels    = NUMBEROFCHANNELS;
	u32 samplePeriodDivisor = SAMPLEPERIODDIVISOR;
	tBoolean continuous     = kTrue;

	int fd, fd2, ic;

	struct timeval origin, before, after[8];

	opterr = 0;
	char c;
	int opt_D=0, opt_o=0, D=0, opt_F=0, opt_T=0, opt_r=80;
	int opt_e=0, opt_I=0, opt_b=-1, opt_B=-1, opt_i=0, adc_range=0;
	int opt_M=0, opt_R=0, opt_at=0, opt_p=1;

	i32 value[8];
	f32 scaled;
	i32 rescaled;
	u32 n = 0;
	int j;
	FILE * param, * calibr[2]={NULL,NULL};
	int cadence;
	struct dma_channel dma;
	static struct scale_table adc_scale, dac_scale[2][2];
        int scan_fd;

	dma.status = DMA_IDLE;

	printf("niconf - starting ...\n");

        /*
         *   get some setup values
         */

        param = fopen(CHANNELS, "r");
                if (!param || fscanf (param, "%d", &numberOfChannels) != 1) {
                        printf ("niconf: could not get channels value\n");
                        exit (-1);
                }
 

	while ((c = getopt (argc, argv, "c:d:r:p:s:x:ItiDoeMRFTb:B:vh")) != -1)
		switch (c)
		{
		case 'c':
			if (optarg) numberOfChannels = atoi(optarg);
			break;
		case 'd':
			if (optarg) samplePeriodDivisor = atoi(optarg);
			break;
		case 'r':
			if (optarg) opt_r = atoi(optarg);
			break;
		case 'I':
			if (opt_i) {
				printf ("%s - the interrupt system will not "
					"be enabled\n", NAM);
				exit (-1);
			}
			opt_I = 1;
			break;
		case 't':
                        opt_at = 1;
                        break;
		case 'i':
			if (opt_I) {
				printf ("%s - the interrupt system will "
					"not be enabled\n", NAM);
				exit (-1);
			}
			opt_i = 1;
			break;
		case 'D':
			opt_D = 1;
			break;
		case 'o':
			opt_o = 1;
			break;
		case 's':
			if (optarg) numberOfSamples = atoi(optarg);
			break;
		case 'p':
			if (optarg) opt_p = atoi(optarg);
			break;
		case 'x':
			if (optarg) adc_range = atoi(optarg);
			break;
		case 'e':
			opt_e = 1;
			break;
		case 'M':
                        opt_M = 1;
                        break;
                case 'R':
                        opt_R = 1;
                        break;
		case 'F':
			opt_F = 1;
			break;
		case 'T':
			opt_T = 1;
			break;
		case 'b':
			opt_b = strtol(optarg, NULL, 0);
			break;
		case 'B':
			opt_B = strtol(optarg, NULL, 0);
			break;
		case 'v':
			D += 1;
			break;
		case 'h':
		default:
			if (c != 'h') {
				fprintf (stderr, "### Unknown option: -%c.\n",
					 optopt);
				fprintf (stderr, display); exit (-1);
			}
			fprintf (stderr, display);
			exit (0);
		}

	/*
	 *    We need get a lock on /dev/spm/scan so nobody can
	 *    modify irq parameters during board configuration.
         *    The lock will be released when niconf exits.
	 */

	scan_fd = open("/dev/spm/scan", O_RDWR | O_NONBLOCK);
        if (scan_fd < 0) {
                printf ("niconf: could not obtain lock on scan: %d\n",
                        scan_fd);
                exit (-1);
        }

	/*
	 *   find and activate the board
	 */

	board[0] = acquire_board("/dev/spm/nibac0", D, &fd);
	if (board[0]) {
		printf ("%s - Found master board as nibac0\n", NAM);

		dma.fd = fd;
		dma.memap[0] = board[0][2] + DMA_CHANNEL_1;
		dma.memap[1] = board[0][3] + DMA_CHANNEL_1;
                if (D>1) {
                        printf ("%s - board mapped at: %p %p %p %p %p\n", NAM,
                                board[0][0], board[0][1], board[0][2],
                                board[0][3], board[0][4]);
                }
        }

	board[1] = acquire_board("/dev/spm/nibac1", D, &fd2);
	if (board[1]) {
		printf ("%s - Found slave board as nibac1\n", NAM);
                if (D>1) {
                        printf ("%s - board mapped at: %p %p %p %p %p\n", NAM,
                                board[1][0], board[1][1], board[1][2],
                                board[1][3], board[1][4]);
                }
	}

        if (board[0] == 0 && board[1] == 0) exit (-1);

	/*
	 *    configure the board
	 */

	if (opt_F || opt_I) {

		/*
		 *   Master board configuration
		 */

                if (board[0]) {

                        /* disable all board interrupts */

                        write32(board[0][2], 0x10, 0x40150000);
		
                        /* stop DMA operation */
		
                        NIBIT (dma.memap, ChannelOperation, write, Stop, 1);
                        gettimeofday(&before, NULL);
                        while (NIBIT(dma.memap, ChannelStatus, read, DmaDone)
                               != 1) {
                                gettimeofday(&origin, NULL);
                                if (usec(&origin, &before) > 100000) {
                                        printf ("Could not get DMA stopped\n");
                                        exit (-1);
                                }
                        }	

                        NIREG (board[0], Interrupt_A_Enable,write, 0);

                        NIBIT (board[0], G0_DMA_Config, set, G0_DMA_Enable, 0);
                        NIBIT (board[0], G0_DMA_Config, write,
                               G0_DMA_Int_Enable, 0);

                        /* disarm board and reset DMA */

                        ai_disarm (board[0]);

                        NIBIT (dma.memap, ChannelOperation, write, Stop, 1);
                        dma.state = DMA_STOPPED; 

                        NIBIT (board[0], AI_AO_Select,set,AI_DMA_Select,0);
                        NIREG (board[0], AI_AO_Select, flush);

                        /* Analog Input reset and configure */

                        board_reset(board[0]);
                        configure_timebase (board[0]);
                        pll_reset (board[0]);
                        analog_trigger_reset (board[0]);
                        ai_reset (board[0]);

                        if (D) printf ("%s - reset and configure completed\n",
                                       NAM);

                        /* setup for adc input */

                        ai_personalize (board[0],
                                        _kAI_CONVERT_Output_SelectActive_High);
                        ai_clear_fifo (board[0]);

                        ai_disarm (board[0]);
    
                        ai_clear_configuration_memory (board[0]);

                        u32 i;
    
                        for (i = 0; i < numberOfChannels; i++) {
                                ai_configure_channel
                                        (board[0], 
                                         i,  /* channel number */
                                         adc_range,  /* gain */
                                         _kAI_Config_PolarityBipolar,
                                         _kAI_Config_Channel_TypeDifferential, 
                                         /* last channel? */
                                         (i == numberOfChannels-1)?
                                         kTrue:kFalse);
                        }

                        ai_set_fifo_request_mode (board[0]);    
    
                        ai_environmentalize (board[0]);
    
                        ai_hardware_gating (board[0]);
    
                        ai_trigger (board[0],
                                    _kAI_START1_SelectPulse,
                                    _kAI_START1_PolarityRising_Edge,
                                    _kAI_START2_SelectPulse,
                                    _kAI_START2_PolarityRising_Edge);
    
                        ai_sample_stop (board[0], (numberOfChannels > 1)?
                                        kTrue:kFalse); /* multi channel? */

                        if (!opt_e) continuous = kFalse;
                        ai_number_of_samples
                                (board[0],
                                 numberOfSamples, /* postrigger samples */
                                 0,               /* pretrigger samples */
                                 continuous);     /* continuous? */

                        param = fopen(
                                "/sys/module/spm_dev/parameters/cadence_usec",
                                "r");
                        if (param) {
                                if (fscanf (param, "%d", &cadence) != 1)
                                        cadence = 0;
                                fclose (param);

                                if (cadence) {
                                        samplePeriodDivisor = cadence * 20;
                                        if (D) printf (
                                                "%s - using divisor %d -> %d"
                                                "usec from cadence_usec\n",
                                                NAM,
                                                samplePeriodDivisor, cadence);
                                }
                        } else {
                                if (D) printf (
                                        "%s - using divisor %d -> %d usec\n",
                                        NAM, samplePeriodDivisor,
                                        samplePeriodDivisor/20);
                        }

                        ai_sample_start (
                                board[0], 
                                samplePeriodDivisor,   /* period divisor */
                                samplePeriodDivisor,   /* 3 , delay divisor */
                                _kAI_START_SelectSI_TC,
                                _kAI_START_PolarityRising_Edge);

                        ai_convert (
                                board[0], 
                                opt_r,  /* 280 */     // convert period divisor
                                3,      // convert delay divisor 
                                kFalse); // external sample clock?

                        ai_clear_fifo (board[0]);

                        if (D) printf ("%s - adc setup completed\n", NAM);

                }

		/*
		 *   read calibration data and configure DAC and digital lines
		 *   on both boards
		 */

		for ( j=0 ; j<2 ; j++ ) {
			if (board[j] == 0) continue;

			/*
			 * read eeprom for calibration information
			 */

			u32 eeprom_size = 1024;

			eeprom_read_MSeries (board[j], board[j][4],
					     eeprom_size);

			if (D>=2) {
                                printf ("\n%s - %s\n\n", NAM,
                                        "Calibration memory content");
				dump_memory (board[j][4], eeprom_size);
			}
                        
			if (D) printf ("%s - eeprom reading completed\n",
					 NAM);

			/*
			 *   analog output reset
			 */

			ao_reset (board[j]);
			ao_personalize (board[j]);
			ao_reset_waveform_channels (board[j]);
			ao_clear_fifo (board[j]);

			/*
			 *   unground AO reference
			 */

			NIBIT(board[j],AO_Calibration,write,
			      AO_RefGround,kFalse);

			/*
			 *   analog output configure
			 */

			ao_configure_dac (board[j],
					  0,
					  0xF,
					  _kAO_DAC_PolarityBipolar,
					  _kAO_Update_ModeImmediate);

			ao_configure_dac (board[j],
					  1,
					  0xF,
					  _kAO_DAC_PolarityBipolar,
					  _kAO_Update_ModeImmediate);

			if (D) printf ("%s - DAC configuration completed "
					 "on board %d at %p\n", NAM, j,
					 board[j]);

			/*
			 *    configure digital IO
			 */

			NIREG (board[j], DIO_Direction, write, 0xff);
		}
        }
        
        /*
         *   configure DMA
         */

        if (opt_R) {
                param = fopen(DMA_USE, "r");
                if (!param || fscanf (param, "%d", &opt_M) != 1) {
                        printf ("niconf: could not get dma_use value\n");
                        exit (-1);
                }
                fclose (param);
		printf ("Keeping DMA channel to state: %d\n", opt_M);

		param = fopen(SAMPLES_PP, "r");
                if (!param || fscanf (param, "%d", &opt_p) != 1) {
                        printf ("niconf: could not get samples value\n");
                        exit (-1);
                }
                fclose (param);
		printf ("Keeping samples to value: %d\n", opt_p);

	}

        if (board[0]) {
                if (opt_I && opt_M) {

                        printf ("%s - starting DMA configuration\n", NAM);

                        NIBIT (board[0], AI_AO_Select,set,AI_DMA_Select,1);

                        NIREG (board[0], AI_AO_Select, flush);

                        dma.mode = DMA_RING;
                        dma.direction = DMA_IN;
                        dma.drq = 0;
                        dma.size = 2 * numberOfChannels * opt_p;
			if (dma.size > 4095) {
                                printf ("niconf: huge DMA buffer! "
                                        "Please, reduce!\n");
                                exit (-1);
                        }
                        dma.transfer_width = DMA_16_BIT;

                        /* read from module parameters the physical address
                           of dma buffer */

                        param = fopen(PHYSICAL_ADDRESS, "r");
                        if (!param || 
                            fscanf (param, "%u",
                                    (u32 *) &dma.physical_address) != 1) {
                                printf ("niconf: dma address not available\n");
                                exit (-1);
                        }
                        fclose (param);

                        dma_configure (&dma);

                        NIBIT (dma.memap, ChannelOperation, write, Start, 1);
                        dma.state = DMA_STARTED;

                        printf ("%s - dma configuration complete\n", NAM);

                } else if (opt_I || opt_F) {              /* deactivate DMA */

                        NIBIT (dma.memap, ChannelOperation, write, Stop, 1);
                        dma.state = DMA_STOPPED; 
                        
                        NIBIT (board[0], AI_AO_Select,set,AI_DMA_Select,0);
                        NIREG (board[0], AI_AO_Select, flush);
                }
        }

        /*
         *   set dma_use flag in module parameters
         */

        if (opt_I || opt_F) {

                param = fopen(DMA_USE, "w");
                if (!param || 
                    fprintf (param, "%c\n", opt_M ? '1' : '0') != 2) {
                        printf ("niconf: could not set dma_use flag\n");
                        exit (-1);
                }
                fclose (param);
        }
        
        /*
         *    activate the interrupt system
         */

        if (opt_I && board[0]) {

                printf ("%s - Activating the interrupt system\n", NAM);
                
                /* enable Mite interrupt IO=1 */
                write32(board[0][2], 0x08, 0x01000000);

		if (opt_M) {

                	/* enable board and DMA interrupt */
                	write32(board[0][2], 0x10, 0x80020000);
                	
//                	NIBIT (board[0], G0_DMA_Config, set, G0_DMA_Enable, 1);
//                	NIBIT (board[0], G0_DMA_Config, write,
//                		G0_DMA_Int_Enable, 1);
                		
		} else {

                	/* enable board interrupt */
                	write32(board[0][2], 0x10, 0x80000000);
                	
	                NIBIT (board[0], Interrupt_Control, write,
        	               Interrupt_Group_A_Enable, 1);

        	        NIBIT (board[0], Interrupt_A_Enable,write,
        	               AI_STOP_Interrupt_Enable, 1);
        	        NIREG (board[0], Interrupt_B_Enable,write, 0);
		}
		
		if (D) printf ("%s - interrupt activation "
        	                       "completed\n", NAM);
	}

 	/*
	 *    read ADC and DAC scale coefficients
	 *
	 */

        if (board[0]) {

                calibr[0] = fopen("/tmp/spm.calibration.master", "w");
                if (!calibr[0]) {
                        printf ("niconf: could not open %s\n",
                                "spm.calibration.master");
                        exit (-1);
                }

                ai_get_scaling_coefficients (board[0][4], adc_range == 4 ? 3 
                                             : adc_range, 0, 0, &adc_scale);

                fprintf (calibr[0], "ADC nibac %d %g  %g  %g  %g\n",
			 adc_scale.order,
                         adc_scale.c[0],adc_scale.c[1],
                         adc_scale.c[2],adc_scale.c[3]);

                if (D>1) printf (
                        "%s - adc scaling coefficients: %d  %g  %g  %g  %g\n",
                        NAM, adc_scale.order,
                        adc_scale.c[0],adc_scale.c[1],
                        adc_scale.c[2],adc_scale.c[3]);
        }

	for ( j=0 ; j<2 ; j++ ) {

                if (D>1) printf ("%s - DAC scale for board %d: %p\n",
                                 NAM, j, board[j]);

		if (board[j] == 0) continue;

                if (j == 1) {
                        calibr[1] = fopen("/tmp/spm.calibration.slave", "w");
                        if (!calibr[1]) {
                                printf ("niconf: could not open %s\n",
                                        "spm.calibration.slave");
                                exit (-1);
                        }
                }

		ao_get_scaling_coefficients (board[j][4], 0, 0, 0,
					     &dac_scale[j][0]);
		ao_get_scaling_coefficients (board[j][4], 0, 0, 1,
					     &dac_scale[j][1]);

                fprintf (calibr[j], "DAC nibac %d  %g  %g  %g  %g\n",
                        dac_scale[j][0].order,
                        dac_scale[j][0].c[0], dac_scale[j][0].c[1],
                        dac_scale[j][0].c[2], dac_scale[j][0].c[3]);

                fprintf (calibr[j], "DAC nibac %d  %g  %g  %g  %g\n",
                        dac_scale[j][1].order,
                        dac_scale[j][1].c[0], dac_scale[j][1].c[1],
                        dac_scale[j][1].c[2], dac_scale[j][1].c[3]);

		printf ("%s - dac 0 scaling coefficients: %d  %g  %g\n",
				NAM, dac_scale[j][0].order,
				dac_scale[j][0].c[0], dac_scale[j][0].c[1]);

		printf ("%s - dac 1 scaling coefficients: %d  %g  %g\n",
				NAM, dac_scale[j][1].order, 
				dac_scale[j][1].c[0], dac_scale[j][1].c[1]);
	}

	if (D>1) printf ("%s - scale coefficients completed\n", NAM);

        if (calibr[0]) fclose (calibr[0]);
	if (calibr[1]) fclose (calibr[1]);

	/*
	 *
	 *    put out bits to digital lines
	 *
	 */

	if (opt_b >= 0 ) {
                if (board[0]) {
                        NIREG (board[0], Static_Digital_Output, write,
                               (u8) 0xff & opt_b);
                        printf ("%s - wrote digital output: 0x%x\n", NAM,
                                NIREG (board[0], Static_Digital_Input, read));
                } else {
                        printf ("%s - %s\n", NAM,
                                "sorry, '-b' option requires a master board");
                }
        }

	if (opt_B >= 0) {
                if (board[1]) {
                        NIREG (board[1], Static_Digital_Output, write,
                               (u8) 0xff & opt_b);
                        printf ("%s - wrote digital output: 0x%x\n", NAM,
                                NIREG (board[1], Static_Digital_Input, read));
                } else {
                        printf ("%s - %s\n", NAM,
                                "sorry, '-B' option requires a second board");
                }
        }

        /*
         *     all done - arm and start the board
         */

        if (opt_at && board[0]) {
                ai_arm (board[0], kTrue);
                ai_start (board[0]);
        }

	/*
	 *
	 *    read adc data from board
	 *
	 */

	if (opt_i && board[0]) {         /* read data polling fifo */

		printf ("%s - going to read ADC from user space\n", NAM);

		/* disable the board interrupt */
		write32(board[0][2], 0x10, 0x40000000);
		usleep (100);

		if (opt_o) printf ("\n");

		gettimeofday(&origin, NULL);
		before = origin;

		while (opt_e || !numberOfSamples ||
		       (n < numberOfChannels*numberOfSamples)) {

			for ( ic=0 ; ic<numberOfChannels ; ic++ ) {
				while (NIBIT(board[0],AI_Status_1,read,
					     AI_FIFO_Empty_St)) {}
				gettimeofday(after+ic, NULL);
				value[ic] = NIREG(board[0],AI_FIFO_Data,read);
			}

			if (!opt_o) printf ("\n");
			printf ("\r%4d     %6.3f  %8ld ",
				n/numberOfChannels,
				msec(&after[0], &origin)/1000.,
				usec(after, &before));
    
			for ( ic=0 ; ic<numberOfChannels ; ic++ ) {
				ai_polynomial_scaler (value+ic, &scaled,
						      &adc_scale);

				if (opt_T && ic)
					printf ("%6ld", usec(after+ic,
							     after+ic-1));

				printf ("%9.3f", scaled);

				if (opt_D && ic < 2) {     /* loop to DAC */ 
					ao_linear_scaler (&rescaled, &scaled,
							  &dac_scale[0][ic]);
					NIREG(board[0], DAC_Direct_Data, ic,
					      write,(*( value+ic))/2);
				}

				n++;

				fflush(NULL);
			}
			before = after[0];
		}
		printf ("\n\n");
	}
        close (scan_fd);
	return 0; 
}
