/*
 *     genevent.c
 *
 *     Generator of dummy events for the 'nobac' driver
 */

 /*
  * Copyright (C) 2008-2009 Marcello Carla'
  *
  * This program is free software; you can use, modify and redistribute
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation (www.fsf.org).
  *
  * Address of the author:
  * Department of Physics, University of Florence
  * Via G. Sansone 1
  * Sesto F.no (Firenze) I 50019 - Italy
  * e-mail: carla@fi.infn.it
  *
  */

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "spm_types.h"
#include "spm_dev_data.h"

#define display \
"Usage: genevent [OPTION]\n"\
"\n"\
" -c <int>   event cadence (usec) [400]\n"\
" -t <int>   top value for the adc scale [20000]\n"\
" -b <int>   bottom value for the adc scale [-20000]\n"\
" -p <int>   pattern period [200]\n"\
" -r <int>   reverse pattern after <int> periods [0]\n"\
" -s <...>   pattern type among: ramp, triangle, square, sin\n"\
" -f <...>   read adc data from user data file\n"\
" -v         print statistics\n"\
" -h         this help\n"

#define PAR_NAMES "c:t:b:p:r:s:f:vh"
#define INFOTIME 2000000
#define SIZE 80

#define PARAMS_DIR "/sys/module/spm_dev/parameters/"

/*
 *   A 'user_function' is available to modify the pseudo-event
 *   to be sent to the interrupt handling routine and the feedback
 *   code; on entry, the 'event' structure is already filled with
 *   the service time and a 'Z' value in adc[0], corresponding to
 *   a fixed pattern. The dac[] fields contain the response given by
 *   the feedback algorithm to the previous event. All fields can be
 *   freely modified.
 *   After the pseudo-event has been processed, the 'cadence_usec'
 *   interval time is added automatically to time[0].
 */

void user_function (struct event_head * event,
		    int16_t * adc,
		    int16_t * dac) {

        int i;

        for ( i=0 ; i<event->samples ; i+=event->n_adc ) {

                adc[1+i] = adc[0+i] / 2;
                adc[2+i] = adc[1+i] / 2;
                adc[3+i] = adc[2+i] / 2;
                adc[4+i] = adc[3+i] / 2;
                adc[5+i] = adc[4+i] / 2;
                adc[6+i] = adc[5+i] / 2;
                adc[7+i] = adc[6+i] / 2;
        }

	for ( i=0 ; i<event->n_dac ; i++ ) {
		if (i >= event->n_adc) break;
		dac[i] = adc[i];
	}
}

int usec(struct timeval * now) {
        return now->tv_sec*1000000 + now->tv_usec;
}

/*
 *    read a parameter value from the /sys filesystem
 */

int read_parameter (char * par_name) {

        int status = 0, value;
        char filename[sizeof(PARAMS_DIR) + strlen(par_name) + 1];
        FILE * param;

        strcpy (filename, PARAMS_DIR);
        strcat (filename, par_name);
        param = fopen(filename, "r");
        if (param) {
                status = fscanf (param, "%d", &value);
                fclose (param);
        }
        if (status != 1) {
                perror ("Error while reading '%s', par_name\n");
                exit (-1);
        }
        return value;
}

int main (int argc, char ** argv) {

        struct params {
                int c;
                int t;
                int b;
                int p;
                int r;
                enum {Ramp, Triangle, Square, Sin} s;
		char * f;
                int v;
        } par = {400,20000,-20000,200,0,Ramp,NULL,0};

        static struct event_head * event, head;
        struct timeval origin, now, next, step, delta;
        float interval, factor;
        int fd;
        int count, partial;
        int sleep_time;
        float steepness;
        int n, l, half;
        char c;
	int timeout;
        int total_sleep;
	int status;
	int samples, is, event_size;
	FILE * data=NULL;
	int16_t *adc, *dac;

        opterr = 0;
        while ((c = getopt (argc, argv, PAR_NAMES)) != -1)
                switch (c) {
                case 'c':
                        par.c = atoi(optarg);
                        break;
                case 't':
                        par.t = atoi(optarg);
                        break;
                case 'b':
                        par.b = atoi(optarg);
                        break;
                case 'p':
                        par.p = atoi(optarg);
                        break;
                case 'r':
                        par.r = atoi(optarg);
                        break;
                case 's':
                        status = strlen(optarg);
                        if (!strncmp(optarg, "ramp", status))
                                par.s = Ramp;
                        else if (!strncmp(optarg, "triangle", status))
                                par.s = Triangle;
                        else if (!strncmp(optarg, "square", status))
                                par.s = Square;
                        else if (!strncmp(optarg, "sin", status))
                                par.s = Sin;
                        else {
                                printf ("Bad pattern! \n %s", display);
                                exit (-1);
                        }
                        break;
		case 'f':
			par.f = optarg;
			break;
                case 'v':
                	par.v = 1;
                	break;
                case 'h':
                default:
                        fprintf (stderr, display);
                        exit (0);
                }

        step.tv_sec = par.c / 1000000;
        step.tv_usec = par.c % 1000000;

        steepness = (float) (par.t - par.b) / par.p;
        half = par.p/2;

        factor = 100./par.c;

        fd = open ("/dev/spm/nobac0", O_RDWR);
        if (fd <= 0) {
                perror ("Open of 'nobac' device failed");
                return -1;
        }

        /*
         *  get actual event parameters
         */

        status = read (fd, (void *)&head + 8, 4);
	if (status != 4) {
		printf ("genevent - read of event parameters failed\n");
		exit (-1);
	}
	printf ("genevent - adc: %d  dac: %d  samples: %d\n",
		head.n_adc, head.n_dac, head.samples);

	/*
	 *  Allocate for the event buffer
	 */

        event_size = 2 * (head.n_dac + (head.n_adc * head.samples)) +
		sizeof(head);
	event = malloc(event_size);
	if (event == 0) {
                printf ("Cannot allocate memory for event buffer\n");
                exit (-1);
        }

	dac = (void *) event+ sizeof(struct event_head);
	adc = dac + head.n_dac;

	/*
	 * data to be taken from user file?
	 */

	if (par.f) {
		data = fopen (par.f, "r");
		if (data == NULL) {
			perror (par.f);
			exit (-1);
		}
	}
	
        count = 0;
        partial = 0;
	timeout = 0;
        total_sleep = 0;
        gettimeofday (&origin, NULL);
        next = origin;

	event->samples = head.samples;
	event->n_adc = head.n_adc;
	event->n_dac = head.n_dac;
        event->r_adc = head.samples * head.n_adc;

	samples = head.n_adc * head.samples;

        while (1) {

        event->irq_time.sec = next.tv_sec;
        event->irq_time.nsec = next.tv_usec * 1000;
        count++;
        partial++;

                if (par.f) {
			for ( is=0 ; is<samples ; is++) {
				status = fscanf (data, " %hd ", adc+is);
				if (status == EOF) {
					if (is) printf ("Odd end of file\n");
					fclose (data);
					close (fd);
					return 0;
				}
			}
		} else {
			for ( is=0 ; is<head.samples ; is++) {

				l = par.r ? (count / (par.p * par.r)) & 1 : 0;
				n = l ? par.p - count % par.p : count % par.p;

				switch (par.s) {
				case Ramp:
					adc[is*head.n_adc] =
						par.b + n * steepness;
					break;
				case Triangle:
					adc[is*head.n_adc] =
						par.b + 2 * steepness *
						(n < half ? n : par.p - n);
					break;
				case Square:
					adc[is*head.n_adc] =
						n > half ? par.b : par.t;
					break;
				case Sin:
					adc[is*head.n_adc] = par.b +
						(par.t - par.b) * 
						(1+sin(2*M_PI*n/par.p))/2; 
					break;
				}
			}

			user_function (event, adc, dac);
		}

                status = write (fd, event, event_size);
                if (status != event_size) {
                        printf ("Event write failed with %d\n", status);
                        exit (-1);
                }

                /* print time statistics */

                gettimeofday (&now, NULL);
                timersub (&now, &origin, &delta);
                interval = usec(&delta);
                if (par.v && interval > INFOTIME) {
                        printf ("sleep %6.2f%% * %dusec - tmo %d\n",
                                factor*total_sleep/partial, par.c, timeout);
                        fflush (stdout);
                        total_sleep = 0;
                        partial = 0;
                        origin = now;
                        timeout = 0;
                }

		/* synchronize to next event */

                timeradd (&next, &step, &next);
                gettimeofday (&now, NULL);
                timersub (&next, &now, &delta);
                sleep_time = usec(&delta);
                total_sleep += sleep_time;
                if (sleep_time < 0) {
                        timeout++;
                } else {
                        usleep (sleep_time);
                }
        }
        return 0;
}
