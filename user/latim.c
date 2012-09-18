/*
 *
 * user/latim.c
 *
 * collect and analyze data on interupt timing
 *
 */

/*
 * Copyright (C) 2008 Marcello Carla'
 *
 * This program is free software; you can use, modify and redistribute
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (www.fsf.org).
 *
 * Address of the author:
 * Department of Physics, University of Florence
 * Via G. Sansone 1
 * Sesto F.no (Firenze) I 50019 - Italy
 * e-mail: <carla@fi.infn.it>
 *
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

#include "latim_pp_data.h"
#include "latim.h"
#include "latim_functions.c"

int main (int argc, char *argv[]) {

	struct params par = PAR_DEFAULT;

	int j, js, fd;
	int n_read;
	int spurious_count=0;
	struct timespec very_first, very_last;
	struct event *time_buf, *time_list, *event;
        struct event *big_time_buf=NULL, origin;
	int side, size;
	int *i_differ, *e_delay=NULL, *i_delay;
	int delta_e;
	int status;
	int count=0, first;
	float sigma, quantum;
	FILE * logfile=NULL, * listfile=NULL;
	FILE * plot=NULL;
	char buff[BUFLEN];
	double *tevi=NULL, delta;
	double cf[2];
	double average;
	int total_time;
	int block_size, full_size;
	int bin_size;

	parse_options (argc, argv, &par);

	if (par.p & MRC_PM) {
		bin_size = (par.n + BINS - 1) / BINS;
		if (par.v) printf ("Bin size is: %d\n", bin_size);
	}
	if (!par.e && par.p & LAT_PM) {
		printf ("The latency distribution will not be computed if"
			" the external clock is not specified [-e]\n");
		par.p &= ~LAT_PM;
	}

	quantum = par.g * par.c;     /* usec/channel in time distributions */

	/* start auxiliary services: gnuplot */

	if (par.p) {
		plot = popen("/usr/bin/gnuplot -noraise -persist -geometry "
			     "1024x512", "w");
		if (plot == NULL) {
			printf ("Start of graphical display failed!\n");
			exit (-1);
		} else {
			if (par.v) printf (
				"Time distributions will be plotted\n");
			fprintf (plot, "set grid; set term X11\n");
		}
	}

	/* open time device and log files */

	if (par.i == NULL) par.i = "/dev/latim";
	if (strcmp(par.i, "-") == 0) {         /* data from standard input */
		fd = 0;
	} else {
		fd = open (par.i, O_RDONLY);
		if (fd == -1) {
			printf ("Failed while opening time device '%s'\n",
				par.i);
			perror ("");
			exit (-1);
		}
	}

	if (!par.f) par.f = new_code();
	logfile = fopen(par.f, "w");
	if (!logfile) {
		perror ("Failed while opening log file");
		exit (-1);
	}
	printf ("Log to file %s\n", par.f);

	if (par.L) {
		strcpy (par.f+strlen(par.f)-3, "lst");
		listfile = fopen (par.f, "w");
		if (!listfile) {
			perror ("Failed while opening list file");
			exit (-1);
		}
		printf ("Time list to file %s\n", par.f);
	}

	/* save the full command line to log file */

	fprintf (logfile, "# ");
	for ( j=0 ; j<argc ; j++) {
		fprintf (logfile, "%s ", *(argv+j));
		if (par.v) printf ("%s ", *(argv+j));
	}
	fprintf (logfile, "\n\n");
	if (par.v) printf ("\n");
	    
	/* get memory for buffers */

	block_size = par.n * SLOT;
	full_size = (par.U ? par.N : 1) * block_size;

	big_time_buf = malloc (full_size+SLOT);
	time_buf = big_time_buf;
	if (!time_buf) {
		perror ("malloc failed with time_buf");
		exit (-1);
	}

	if (par.v) printf ("Allocated time buffer %d bytes wide at %p\n",
			   (int) (full_size+SLOT), big_time_buf);

	time_list = time_buf + 1;

	tevi = malloc (par.n * sizeof(double));
	if (!tevi) {
		perror ("malloc failed with tevi");
		goto bad_exit;
	}
	if (par.v) printf ("Allocated count buffer %d bytes wide at %p\n",
			   (int) (par.n * sizeof(*tevi)), tevi);

	side = par.m/par.c + 1.0;
	size = 2*side + 1;

	e_delay = calloc (3*size, sizeof(int));
	if (!e_delay) {
		perror ("malloc failed with e_delay");
		goto bad_exit;
	}
	if (par.v) printf ("Allocated distribution buffer %d bytes wide "
			   "at %p\n", (int) (3*size*sizeof(int)), e_delay);
	i_differ = e_delay + size;
	i_delay = i_differ + size;

	/*
	 *   enter the main loop
	 */

	n_read = read (fd, (void *) time_buf, SLOT);
	if (n_read != SLOT) {
		printf ("reading of time origin failed with %d\n", n_read);
		goto bad_exit;
	}

	origin = *time_buf;
	count = 0;
	while (!par.N || count < par.N) {

		n_read = 0;
		while (n_read < block_size) {
			status = read (fd, (void *) time_list +
				       n_read, block_size - n_read);
			if (status == 0) break;
			if (status < 0) {
				perror ("read operation failed");
				goto bad_exit;
			}
			n_read += status;
		}
		if (status == 0) {
			printf ("Undue EOF after %d bytes\n", n_read);
			goto bad_exit;
		}

		if (n_read != block_size) {
			printf("read operation failed with return %d\n",
			       n_read);
			perror ("reason");
			goto bad_exit;
		}
    
		first = count * par.n;

		/*
		 *    issue a complete list of all events:
                 *    -l  list to console
                 *    -L  list to file
		 */

                list_events (&par, listfile, time_list, first, &origin);

		/*     Analyze buffer for spurious events.
		 *     An event is spurious when the measured latency exceeds
		 *     the given threshold (-m).
		 */

                if (check_events (&par, time_list, logfile, count,
                                  &spurious_count, first, &origin) == -1)
                  goto bad_exit;
                
		printf ("events/err.: %9d / %d", first+par.n, spurious_count);

		/*
		 *    histograms:
		 *
		 *    i_differ: distribution of internal clock differences
		 *    e_delay:  distribution of external counter delays
		 *    i_delay:  distribution of internal clock delays
		 *
		 */

		/* build i_differ and e_delay time distributions */

		for ( j=0 ; j<par.n ; j++ ) {
			event = time_list+j;
			delta = (us_diff(&event->timic, &(event-1)->timic) -
                                 par.t) / quantum;
			if (delta < -side) delta = -side;
			else if (delta > side) delta = side;
			i_differ[(int) (delta + side)] += 1;
//			tevi[j] = us_diff(&event->timic, &origin.timic);
			tevi[j] = us_diff(&event->timic, &time_list->timic);
			if (par.e) {
				delta_e = diff_e(event) / par.g;
				if (delta_e > size) delta_e = size;
				e_delay[delta_e] += 1;
			}
		}

		/* compute clock rate and build 'i_delay' histogram */

		fit1 (tevi, par.n, cf);
		printf ("      clock rate: %9.3f  %8.3f\n", cf[0], cf[1]+200);

		for ( j=0 ; j<par.n ; j++ ) {
			tevi[j] -= (cf[0]*(j+1) + cf[1]);
			if (tevi[j] < -par.m) tevi[j] = -par.m;
			if (tevi[j] >= par.m) tevi[j] = par.m;
			delta = tevi[j] / (par.g * par.c);
			i_delay[(int) (delta+side)] += 1;
		}

		/* plots */

                plot_histograms
                  (&par, plot, i_differ, i_delay, e_delay, tevi, time_list);

                /*
                 * close the main loop
                 */

		if (!count) very_first = time_buf->timic;
		very_last = (time_buf + par.n)->timic;
		if (par.U) {
			time_buf += par.n;
			time_list = time_buf + 1;
		} else {
			*time_buf = *(time_buf + par.n);
		}
		count++;
	}

	/* print final logs */

	if ((par.L || par.l) && par.U) {
		for ( j=0 ; j<par.n*par.N ; j++ ) {
			event = big_time_buf+j+1;
			delta = us_diff (&event->timic, &(event-1)->timic);
			sprintf (buff, "%8d  %12ld.%09ld  %10d  %8.2f %6.2f",
                                 j, event->timic.tv_sec, event->timic.tv_nsec,
				 tm_diff(&event->timic, &origin.timic),
				 delta, delta - par.t);
			if (par.e) {
				delta_e = diff_e(event);
				sprintf (buff+strlen(buff), "    %4d  %4d",
					 event->timec, delta_e);
			}
			sprintf (buff+strlen(buff), "\n");
			if (par.l) printf (buff);
			if (par.L) fprintf (listfile, buff);
		}
	}

	/* print final time distributions */

	if (par.S) {                                        /* i_differ */
		printf ("\nTime distribution from internal clock\n");
		print_distrib (i_differ, size, logfile, side, quantum);

		if (par.e) {                                 /* e_delay */
			printf ("\nTime distribution from external clock\n");
			fprintf (logfile, "\n\n");
			print_distrib (e_delay, size, logfile, 0, quantum);
		}

		printf ("\nTime distribution of delays from internal clock\n");
		fprintf (logfile, "\n\n");                       /* delay */
		print_distrib (i_delay, size, logfile, side, quantum);
	}

	/* time distribution average and width */

	if (par.s || par.S) {

		sigma = 0.;
		average = 0.;

		for ( js=-side ; js<=side ; js++ )
                  average += i_differ[js+side] * js;

		average /= (par.n * par.N)/quantum;

		for ( js=-side ; js<=side ; js++ )
                  sigma += i_differ[js+side] * (js-average) * (js-average);

		sprintf (buff, "par.t: %d   <T>: %f   Sigma: %f\n", par.t,
			 average, quantum*sqrt(sigma/(par.n * par.N - 1)));

		printf ("\n%s", buff);
		fprintf (logfile, "\n# %s", buff); 
	}

	total_time = tm_diff(&very_last, &very_first);
	sprintf (buff, "Total time: %d   skew: %d\n",
		 total_time, total_time -
		 par.n * par.N * par.t);
	printf (buff);
	fprintf (logfile, "# %s", buff);

	if (listfile) fclose (listfile);
	if (logfile) fclose (logfile);

	if (par.v) printf ("Cleaning buffers at %p %p %p\n",
			   big_time_buf, tevi, e_delay);
	free (big_time_buf);
	free (tevi);
	free (e_delay);

	return 0;

 bad_exit:
	if (logfile) fclose (logfile);
	if (listfile) fclose (listfile);
	free (big_time_buf);
	free (tevi);
	free (e_delay);
	return -1;
}
