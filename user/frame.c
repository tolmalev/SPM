/*
 *     frame.c
 *
 *     A sample program to get a frame scanned with the 'spm' package
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
 * e-mail: carla@fi.infn.it
 *
 */

#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <strings.h>
#include <signal.h>

#include "spm_types.h"
#include "spm_dev_data.h"
#include "latim_pp_data.h"
#include "frame.h"

#define TCL_VIEWER SPM_DIR"/user/show"
#define IMAGE_FILE "/dev/shm/spm_image.pnm"
#define LATIM SPM_DIR"/user/latim"

int go_exit=0;

/*
 * catch_the_signal(signal) - termination signals
 */

void catch_the_signal (signal) {
	printf ("Received signal %d\n", signal);
	go_exit = 1;
	return;
}

int main (int argc, char ** argv) {

	struct params par = PAR_DEFAULT;

	int fd, status;
	int linesize;
	struct event_head *line_buf, *inbuf, *event;
	int i, points_per_frame;
	int samples_per_frame;
	int line_count;
	struct timeval origin, before, after, very_first, very_last;
	FILE * plot=NULL, * latim=NULL;
	int latim_fd;
	char * datafile;
	int data_fd;
	int ibuf=0;
	int image_fd;
	char var_buf[256];
	int var_buf_size;
	int shrink;
	int points_per_image=512;
	FILE * viewer=NULL;
	int total_time;
	int times_buf_size;
	struct sigaction act;
	struct scan_params command;
	struct event * times=NULL;
	struct statistics st;
	int slot_size;

	printf ("frame - get a frame scan with the 'spm' package\n");

	parse_options (argc, argv, &par);

	/*
	 *   setup to intercept SIGINT signal
	 */

	act.sa_handler = catch_the_signal;
	sigemptyset(&act.sa_mask);    /* exclude all signals from block mask */
	act.sa_flags = 0;             /* behaviour flag: none */
	sigaction(SIGINT, &act, 0);
	sigaction(SIGPIPE, &act, 0);
	sigaction(SIGHUP, &act, 0);
	sigaction(SIGALRM, &act, 0);
	sigaction(SIGTERM, &act, 0);

	/*
	 *   read parameters as known to the kernel module
	 */


        command.sample_adc = read_parameter ("sample_adc");
        command.sample_dac = read_parameter ("sample_dac");
	command.samples_per_point = read_parameter ("samples_per_point");

	printf ("frame - adc: %d  dac: %d  samples: %d  ch: %d\n",
		command.sample_adc, command.sample_dac,
		command.samples_per_point, par.C);

/*
	if ((par.C + command.sample_dac) < 0  || 
	    (par.C > (command.sample_adc-1) * command.samples_per_point)) {
        	printf ("`-C %d` is out of allowed range\n", par.C);
                exit (-1);
        }
*/
	command.points_per_line = read_parameter ("points_per_line");
	command.lines_per_frame = read_parameter("lines_per_frame");
	command.cadence_usec = read_parameter("cadence_usec");

        slot_size = sizeof(struct event_head) + 2 *
		(command.sample_adc * command.samples_per_point +
		 command.sample_dac);

	/*
	 *   prepare the scan command
	 */

	command.code[0] = 's';
	command.mask = 0;

	if (par.a > 0) {
		command.buffer_size_min = par.a;
		command.mask |= BUFFER_SIZE_MIN;
	}
	if (par.b > 0) {
		command.buffer_size = par.b;
		command.mask |= BUFFER_SIZE;
	}
	if (par.c > 0) {
		command.cadence_usec = par.c;
		command.mask |= CADENCE_USEC;
	}
	if (par.l >= 0) {
		command.lines_per_frame = par.l;
		command.mask |= LINES_PER_FRAME;
	}
	if (par.p >= 0) {
		command.points_per_line = par.p;
		command.mask |= POINTS_PER_LINE;
	}
	if (par.t > 0) {
		command.timeout = par.t;
		command.mask |= TIMEOUT;
	}
	if (par.w > 0) {
		command.high_water = par.w;
		command.mask |= HIGH_WATER;
	}

	/* compute global parameters */

	if (command.points_per_line < 1) {
		printf ("points_per_line must be greater than zero;"
			" it is %d\n", command.points_per_line);
		exit (-1);
	}
	if (command.lines_per_frame < 0) {
		printf ("lines_per_frame must be greater or equal zero;"
			"it is %d\n", command.lines_per_frame);
		exit (-1);
	}

	linesize = command.points_per_line * slot_size;
	if (par.v) printf ("linesize = %d\n", linesize);

	printf ("Going to take %d samples x %d points x %d lines\n"
                "with cadence %d us and event size %d bytes\n",
                command.samples_per_point, command.points_per_line,
		command.lines_per_frame, command.cadence_usec,
                slot_size);

	/*
	 *    start auxiliary services: gnuplot and latim
	 */

	if (par.L) {
		plot = popen("/usr/bin/gnuplot -persist -noraise", "w");
		if (plot == NULL) {
			printf ("Start of graphical display failed!\n");
			exit (-1);
		}
		printf ("Data will be plotted\n");
		fprintf (plot, "set term x11 0;set styl dat lin; set grid\n");
	}

	if (par.D) {
		times_buf_size =
			command.points_per_line*sizeof(struct event_times);
		var_buf_size = snprintf
			(var_buf, 256, LATIM" -i - %s\n", par.D);
		if (var_buf_size == 256) {
			printf ("Command line overflow\n");
			par.D = 0;
		} else {
			latim = popen(var_buf, "w");
			if (latim == NULL) {
				printf ("Start of time analyzer failed!\n");
				par.D = 0;
			} else {
				latim_fd = fileno(latim);
				printf ("Service times will be analyzed "
					"on fd %d\n", latim_fd);
			}
		}
	}
  
	/*   open file for data log   */

	if (par.S) {
		datafile = new_code();
		data_fd = open
			(datafile, O_RDWR | O_CREAT | O_EXCL, 0644);

		if (!data_fd) {
			perror ("Open of data file failed");
			exit (-1);
		}
		printf ("Data  will be written to file %s\n", datafile);
		printf ("with event size %d\n", slot_size);
	}

	/* allocate buffer for last n lines, n=1 if no line display */

	line_buf = calloc(par.L ? par.L : 1, linesize);
	if (line_buf == 0) {
		perror ("Memory allocation for line buffer failed");
		exit (-1);
	}
	if (par.v) printf ("frame - line buffer at %p\n", line_buf);

	/* allocate buffer for latency distribution */

	if (par.D) {
		times = malloc(sizeof(struct event_times) * 
			       command.points_per_line);
		if (times == NULL) {
			printf ("malloc for time analysis failed!\n");
			par.D = 0;
			pclose (latim);
		}
	}

	/*   create and populate pnm image file and start the viewer */

	if (par.F) {

		if (command.lines_per_frame == 0) {
			printf ("Frame display not supported with "
				"endless loop\n");
			par.F = 0;
			goto end_viewer;
		}
		var_buf_size = snprintf (var_buf, 256,
					 "P5\n#scan image\n%d %d\n%d\n",
					 points_per_image,
					 command.lines_per_frame, 255);

		image_fd = open (IMAGE_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (!image_fd) {
			perror ("Error while opening file for image display");
			par.F = 0;
			goto end_viewer;
		}
		write (image_fd, var_buf, var_buf_size);
		posix_fallocate(image_fd, var_buf_size,
				command.lines_per_frame * points_per_image);
		shrink = (command.points_per_line + points_per_image - 1) /
			points_per_image;
		printf ("Image to file %s - shrink factor: %d\n",
			IMAGE_FILE, shrink);

		/* open the image viewer */

		viewer = popen("wish -geometry -0+0 "IMAGE_FILE, "w");
		if (viewer == NULL) {
			printf ("Start of image viewer failed!\n");
			close (image_fd);
			goto end_viewer;
		} else {
			printf ("Image will be shown\n");
			fprintf (viewer, "source "TCL_VIEWER"\n");
			fflush (viewer);
			sleep (1);
		}
	}
  
 end_viewer:
 
	/*
	 *    open the 'scan' device and start operations
	 */

	fd = open ("/dev/spm/scan", O_RDWR);

	if (fd <= 0) {
		perror ("Open of 'scan' device failed");
		return -1;
	}

	status = write (fd, command.code, sizeof(command));
	if (status != sizeof(command)) {
		perror ("Request for scan failed");
		exit (-1);;
	}
	printf ("Requested start of scan\n");

	/*
	 *    data loop
	 */

	gettimeofday (&origin, NULL);

	line_count = 0;

	while (1) {
  
		if (go_exit) break;

		/* fill buffer */

		inbuf = (void *) line_buf + ibuf * linesize;

		status = get_a_line ((u8 *) inbuf, linesize, fd);

		if (status < linesize) {
			printf ("Eof after %d lines\n", line_count);
			break;
		}

		/* update progress counter to console */

		gettimeofday (&before, NULL);
		printf ("line %d\r", line_count);
		fflush(stdout);

		/* build statistics and check for error conditions */

                statistics (inbuf, &st, command.points_per_line,
			    slot_size);

		/* store data to file */

		if (par.S) {
			if (par.v > 1)
				printf ("Writing %d bytes at %p to fd %d\n",
					linesize, inbuf, data_fd);
			if (write (data_fd, inbuf, linesize) == -1)
				perror ("write");
		}

		/* create  and display image */

		if (par.F) {
			create_image (inbuf, &command, image_fd,
				points_per_image, shrink, par.C,
				slot_size); 

			fprintf (viewer, "update\n");
			fflush (viewer);
		}

		/* line plots */

		if (par.L > 0) {
			line_plot (plot, line_count, &par, ibuf, line_buf,
				   &command, slot_size);
		}

		/* send time data for latency analysis to 'latim'*/

		if (par.D) {
			for (i=0 ; i<command.points_per_line ; i++) {
                                /*
				(times+i)->irq_time = IVENT(inbuf,i)->irq_time;
                                */
                                (times+i)->timic.tv_sec = 
                                        IVENT(inbuf,i)->irq_time.sec;
                                (times+i)->timic.tv_nsec = 
                                        IVENT(inbuf,i)->irq_time.nsec;
			}

			write (latim_fd, times, times_buf_size);
		}

		/* save the start and stop time of frame scan */

		if (!line_count) {
			very_first.tv_sec = inbuf->irq_time.sec;
			very_first.tv_usec = inbuf->irq_time.nsec/1000;
		}
		
		event = IVENT(inbuf,command.points_per_line-1);
		very_last.tv_sec = event->irq_time.sec;
		very_last.tv_usec = event->irq_time.nsec / 1000;

		line_count++;
		if (par.L>0) ibuf = (ibuf+1) % par.L;
		gettimeofday (&after, NULL);
		if (par.v) printf  ("line %d - process time: %7.1f msec\n", 
				    line_count, usec(&after, &before)/1000.);
	}

	total_time = usec(&very_last, &very_first);
	points_per_frame = command.points_per_line * line_count;
	samples_per_frame = points_per_frame * command.samples_per_point;

	printf ("Total time: %10.6f sec    skew: %d usec\n",
		total_time/1.0e6, total_time - command.cadence_usec *
		(samples_per_frame -1));

	printf ("ADC times (usec)      [min avg max]: %8.1f  %8.1f  %8.1f\n",
		st.adc.min/100., st.adc.avg/(100. * points_per_frame),
		st.adc.max/100.);
	printf ("Service times (usec)  [min avg max]: %8.1f  %8.1f  %8.1f\n",
		st.service.min, st.service.avg/points_per_frame,
                st.service.max);
	printf ("Interval times (usec) [min avg max]: %8.1f  %8.1f  %8.1f\n",
		st.interval.min, st.interval.avg/(points_per_frame-1),
                st.interval.max);

	close (fd);
	fdatasync (data_fd);
	close (data_fd);

	return 0;
}
