#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "spm_types.h"
#include "spm_dev_data.h"
#include "frame.h"
#include "calibration.h"

/*
 *    parse command line options
 */

void parse_options (int argc, char *argv[], struct params *par) {

	char c;
	int j;
	opterr = 0;
	while ((c = getopt (argc, argv, PAR_NAMES)) != -1)
		switch (c)
		{
		case 'a':
			par->a = atoi(optarg);
			break;
		case 'b':
			par->b = atoi(optarg);
			break;
		case 'c':
			par->c = atoi(optarg);
			break;
		case 'h':
		case '?':
			fprintf (stderr, display);
			exit (0);
		case 'l':
			par->l = atoi(optarg);
			break;
		case 'p':
			par->p = atoi(optarg);
			break;
		case 't':
			par->t = atoi(optarg);
			break;
		case 'v':
			par->v +=1;
			if (par->v > 1) printf ("Debug now active!\n");
			break;
		case 'w':
			par->w = atoi(optarg);
			break;
		case 'd':
			par->d = 1;
			break;
		case 'D':
			par->D = optarg;
			break;
		case 'F':
			par->F = 1;
			break;
		case 'C':
			par->C = atoi(optarg);
			break;
		case 'R':
			par->R = 1;
			break;
		case 'L':
			par->L = atoi(optarg);
			break;
		case 'S':
			par->S = 1;
			break;
		default:
			fprintf (stderr, "### Unknown option: -%c.\n", optopt);
			fprintf (stderr, display);
			exit (-1);
		}
	if (optind != argc) {
		for (j = optind; j < argc; j++) {
			fprintf (stderr, "Unknown argument: <%s>\n", argv[j]);
		}
		fprintf (stderr, display);
		exit (-1);
	}
}

/*
 *    build a time-stamped measure code
 */

char * new_code () {
	static char code[28];
	time_t now;
	struct tm * t;
	now = time(NULL);
	t = localtime(&now);
	sprintf (code, "log-%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d.dat",
		 t->tm_year+1900, t->tm_mon+1, t->tm_mday,
		 t->tm_hour, t->tm_min, t->tm_sec);
	return code;
}

/*
 *    usec - difference in microseconds between two timeval times
 */

long int usec(struct timeval * after, struct timeval * before) {
	return ((after->tv_sec - before->tv_sec) * 1000000 +
		(after->tv_usec - before->tv_usec));
}

/*
 *   usec_diff : difference (in usec) between times in internal spm format
 */

double usec_diff (struct irq_time * a, struct irq_time * b) {
	return ((a->sec - b->sec)*1.0e6 +
		(a->nsec - b->nsec)/1.0e3);
}

struct timespec * ns_diff (struct timespec * a,
                           struct timespec * b,
                           struct timespec * d) {
	d->tv_sec = a->tv_sec - b->tv_sec;
	d->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (d->tv_nsec < 0) {
		d->tv_sec--;
		d->tv_nsec +=1000000000;
	}
	return d;
}

double ns2us (struct timespec * d) {
	return (d->tv_sec*1.0e6 + d->tv_nsec/1.0e3);
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
		printf ("Error while reading %s\n", par_name);
		exit (-1);
	}
	return value;
}

/*
 *     get_a_line - iterate the read request until buffer is full or EOF
 */

int get_a_line (u8 * inbuf, int linesize, int fd) {

	int fill=0, status;

	while (fill < linesize) {
		status = read (fd, inbuf+fill, linesize-fill);
		if (status == 0) return fill;
		if (status < 0) {
			perror ("read operation failed");
			exit (-1);
		}
		fill += status;
	}
	return fill;
}

/*
 *      build_statistics and check for error conditions
 */

void statistics (struct event_head * inbuf, struct statistics * st,
					 int n, int slot_size) {

	static struct irq_time last_time={0,0};
	struct event_head * point;
	double delta;
	int i;

	/* on the very first event clear all accumulators */

	if (last_time.sec == 0) {
		st->fifo_errors = 0;
		st->adc.min = 1e99;
		st->adc.avg = 0;
		st->adc.max = 0;
		st->service.min = 1e99;
		st->service.avg = 0;
		st->service.max = 0;
		st->interval.min = 1e99;
		st->interval.avg = 0;
		st->interval.max = 0;
		last_time = inbuf->irq_time;
		i = 1;
	} else {
		i = 0;
	}

	for ( ; i < n ; i++) {

		point = J_SLOT(inbuf,i);

		if (point->r_adc != point->n_adc * point->samples) {
			printf ("FIFO ERROR at point %d: %d\n",
				i, point->r_adc);
			st->fifo_errors += 1;
		}

		delta = point->adc_time;
		st->adc.avg += delta;
		if (delta > st->adc.max) st->adc.max = delta;
		if (delta < st->adc.min) st->adc.min = delta;

		delta = point->service_time;
		st->service.avg += delta;
		if (delta > st->service.max) st->service.max = delta;
		if (delta < st->service.min) st->service.min = delta;
		
		delta = usec_diff(&point->irq_time, &last_time);
		st->interval.avg += delta;
		if (delta > st->interval.max) st->interval.max = delta;
		if (delta < st->interval.min) st->interval.min = delta;

		last_time = point->irq_time;
	}
}

/*
 *     create image for on-line frame display
 */

void create_image (struct event_head * inbuf,         /* las line buffer */
		   struct scan_params * command, /* frame parameters */
		   int image_fd,                 /* image file fd */
		   int points_per_image,         /* image line size */
		   int shrink,                   /* horiz. shrink factor */
		   int adc,                      /* adc channel to be used */
		   int slot_size) {
		   
	int ival, i, j, is;
	struct event_head * this, * next;
	int16_t * vec;
        float val[points_per_image];
        int blank;
	float min, max, span;

	/* build the line vector */

	if (shrink <= 1) {
		for ( i=0 ; i<command->points_per_line ; i++) {
			this = IVENT(inbuf, i);
			next = IVENT(inbuf, i+1);
			vec = (int16_t *) next - this->n_adc * this->samples;
			val[i] = vec[adc];
		}
		blank = i;
	} else {
		j = 0;
		for ( i=0 ; i < command->points_per_line ; i+=shrink,j++) {
			val[j] = 0;
			for ( is=0 ; is < shrink ; is++ ) {
				this = IVENT(inbuf, i+is);
				next = IVENT(inbuf, i+is+1);
				vec = (int16_t *)
					next - this->n_adc * this->samples;
				val[j] += vec[adc];
			}
			val[j] /= shrink;
		}
		blank = j;
	}

	/* normalize */

	min = max = val[0];
	for ( i=0 ; i<blank ; i++ ) {
		if (val[i] < min) min = val[i];
		if (val[i] > max) max = val[i];
	}
	span = 255 / (max - min);

	for ( i=0 ; i<blank ; i++ ) {
		ival = (val[i] - min) * span;
		write (image_fd, &ival, 1);
	}
	ival = 0;
	for ( ; i<points_per_image ; i++ ) {
		write (image_fd, &ival, 1);
	}

}

/*
 *    plot last lines
 */

void line_plot (FILE * plot,                    /* stream to gnuplot */
		int line_count,                 /* how many lines */
		struct params * par,            /* command line parameters */
		int ibuf,                       /* index of last line */
		struct event_head * line_buf,   /* line buffer */
		struct scan_params * command,   /* frame parameters */
		int slot_size) {

	int i, j, k, nl;
	struct event_head * plot_buf, * point;
	int16_t adc;

        nl = (line_count+1 >= par->L) ? par->L : line_count+1;

	fprintf (plot, "plot '-' tit '%d'", line_count);
	for ( j = 1 ; j < nl ; j++)
		fprintf (plot, ",'-' tit '%d'", line_count-j);
	fprintf (plot, "\n");

	for ( j = 0 ; j < nl ; j++ ) {
		
		plot_buf = (void *) line_buf +
			command->points_per_line * slot_size *
			((ibuf + par->L - j) % par->L);

		if (par->v >= 1) printf ("plotting line %d %d at %p\n",
					 ibuf, j, plot_buf);
		
		for ( i=0 ; i < command->points_per_line ; i++) {

			point = J_SLOT(plot_buf,i);

			for ( k = 0 ; k < command->samples_per_point ; k++ ) {
					
				adc = SAMPLE(point,k)[par->C];
 
				if (par->R) {
					fprintf (plot, "%6g\n",
						 scale_adc_data(adc));
				} else {
					fprintf (plot, "%6d\n", adc);
				}
				if (par->C < 0) break;
			}
		}
		fprintf (plot, "e\n");
	}
	fflush (plot);
}
