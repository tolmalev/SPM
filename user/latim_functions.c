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
			par->a = 1;
			break;
		case 'B':
			par->B = 1;
			break;
		case 'c':
			par->c = atof(optarg);
			break;
		case 'E':
			par->E = 1;
		case 'e':
			par->e = 1;
			break;
		case 'f':
			par->f = optarg;
			break;
		case 'g':
			par->g = atoi(optarg);
			break;
		case 'h':
		case '?':
			fprintf (stderr, display);
			exit (0);
		case 'i':
			par->i = optarg;
			break;
		case 'l':
			par->l = 1;
			break;
		case 'L':
			par->L = 1;
			break;
		case 'm':
			par->m = atof(optarg);
			break;
		case 'n':
			par->n = atoi(optarg);
			if (par->n < 1) {
				fprintf (stderr, "read - n: illegal request "
					 "(%s)\n", optarg);
				exit (-1);
			}
			if (par->v) printf ("D - requested %d events\n",
					    par->n);
			break;
		case 'N':
			par->N = atoi(optarg);
			break;
		case 'p':
			par->p = atoi(optarg);
			break;
		case 's':
			par->s = 1;
			break;
		case 'S':
			par->S = 1;
			break;
		case 't':
			par->t = atoi(optarg);
			if (par->t < 1) {
				fprintf (stderr, "read - n: illegal request "
					 "(%s)\n", optarg);
				exit (-1);
			}
			if (par->v) printf ("D - central time: %d\n", par->t);
			break;
		case 'U':
			par->U = 1;
			break;
		case 'v':
			par->v +=1;
			if (par->v > 1) printf ("Debug now active!\n");
			break;
		case 'x':
			par->x = atoi(optarg);
			break;
		default:
			fprintf (stderr, "### Unknown option: -%c.\n", optopt);
			fprintf (stderr, display);
			exit (-1);
		}
	if (optind != argc) {
		for (j = optind; j < argc; j++) {
			fprintf (stderr, "### Unknown argument: <%s>\n",
				 argv[j]);
		}
		fprintf (stderr, display);
		exit (-1);
	}
}

/*
 *    time functions
 */

int tm_diff (struct timespec *a, struct timespec *b) {
	return ((a->tv_sec - b->tv_sec)*1000000 +
		(a->tv_nsec - b->tv_nsec)/1000);
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

double us_diff (struct timespec * a, struct timespec * b) {
	return ((a->tv_sec - b->tv_sec)*1.0e6 +
		(a->tv_nsec - b->tv_nsec)/1.0e3);
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
	sprintf (code, "ltm-%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d.log",
		 t->tm_year+1900, t->tm_mon+1, t->tm_mday,
		 t->tm_hour, t->tm_min, t->tm_sec);
	return code;
}

/*
 *   conditional print of a time value
 */

char * if_timec (int flag, int time) {
	static char *nihil="";
	static char value[5];
	if (flag) return nihil;
	sprintf (value, "%4d %4x", time, time);
	return value;
}

/*
 *   first and last non-zero value in vector
 */

int first_non_zero (int * vector, unsigned int length, unsigned int * last) {
	int j;
	j = length;
	while (j && !vector[--j]);
	*last = j;
	j = -1;
	while (++j < *last && !vector[j]);
	return (j);
}

/*
 *   print a time distribution
 */

void print_distrib (int * distr,
		    int size,
		    FILE * logfile,
		    int offset,
		    float quantum) {

	int js;
	unsigned int last_channel;
	char buff[BUFLEN];

	js = first_non_zero(distr, size, &last_channel);

	for ( ; js<=last_channel ; js++ ) {
		sprintf (buff, "%f  %8d\n", (js-offset)*quantum, distr[js]);
		if (distr[js]) printf (buff);
		fprintf (logfile, buff);
	}
}

/*
 *     least squares fit with first degree polynomial
 */

void fit1 (double * y, int n, double * cf) {

	int j;
	double sn = n;
	double sumx = 0.;
	double sumy = 0.;
	double sumx2 = 0.;
	double sumxy = 0.;
	double sx = 0.;
	double sy = 0.;
	double sqsumx, smxsmy;
	double ds, da, db;

	for ( j=0 ; j<n ; j++) {
		sx = j+1;
		sy = y[j];
		sumx += sx;
		sumy += sy;
		sumx2 += sx*sx;
		sumxy += sx*sy;
	}

	sqsumx = sumx * sumx;
	smxsmy = sumx * sumy;

	ds = sn * sumx2 - sqsumx;
	da = sn * sumxy - smxsmy;
	db = sumx2* sumy - sumxy * sumx;

	cf[0] = da/ds;
	cf[1] = db/ds;

	return;
}

/*
 *      show a complete list of all events
 */

void list_events (struct params * par,
                  FILE * listfile,
                  struct event * time_list,
		  int first,
		  struct event * origin) {

	int j, delta_e;
	struct event * event;
	double delta;
	char buff[BUFLEN];

	if ((!par->l && !par->L) || par->U) return;

	if (par->B && par->L && first) fprintf (listfile, "\n");
	for ( j=0 ; j<par->n ; j++ ) {
		event = time_list+j;
		delta = us_diff (&event->timic,
				 &(event-1)->timic);
		sprintf (buff, "%8d  %12ld.%09ld  %10d  %8.2f %6.2f",
			 first+j,
			 (time_list+j)->timic.tv_sec,
			 (time_list+j)->timic.tv_nsec,
			 tm_diff(&event->timic, &origin->timic),
			 delta, delta - par->t);
		if (par->e) {
			delta_e = diff_e(event);
			sprintf (buff+strlen(buff), "    %4d  %4d",
				 event->timec, delta_e);
		}
		sprintf (buff+strlen(buff), "\n");
		if (par->l) printf (buff);
		if (par->L) fprintf (listfile, buff);
	}
}

/*
 *     Analyze buffer for spurious events.
 *     An event is spurious when the measured latency exceeds
 *     the given threshold (-m).
 */

int check_events (struct params * par,
		   struct event * time_list,
		   FILE * logfile,
		   int count,
		   int * spurious_count,
		   int first,
		   struct event * origin) {

        int j;
	int delta_i, delta_e;
	struct event * event;
	int anomaly=0;
	char buff[BUFLEN];
	int anomal_e=0;

	if (!par->a) return 0;

	for ( j=0 ; j<par->n ; j++ ) {

		/* check event j for anomaly */

		event = time_list+j;
		delta_i = tm_diff(&event->timic,
				  &(event-1)->timic) - par->t;
		delta_e = diff_e(event);
		if (delta_i < -par->m || delta_i > par->m) {
			anomaly |= 1;
		}
		if (par->e && (par->c * delta_e > par->m)) {
			anomaly |= 1;
			anomal_e +=1;
		}

		/* if anomaly in event j, first print event j-1 */

		if (anomaly == 1) {

                        sprintf (buff, "#   event number             "
                                 "event time          "
                                 "interval delay    d counter x\n");
                        printf (buff);
			fprintf (logfile, buff);

			sprintf (buff, "# %8d+%-8d   %12ld.%09ld %20s %s\n",
				 j-1, first, (event-1)->timic.tv_sec,
				 (event-1)->timic.tv_nsec, " ",
				 if_timec(!par->e, (event-1)->timec));
			printf (buff);
			fprintf (logfile, buff);
		}

		/* if anomaly in event j or event j-1, print event j */

		if (anomaly) {
			sprintf (buff,
				 "# %8d+%-8d   %12ld.%09ld  %10d %6d   %s\n",
				 j,first, event->timic.tv_sec,
				 event->timic.tv_nsec, delta_i + par->t,
				 delta_i, if_timec(!par->e, event->timec));
			printf (buff);
			fprintf (logfile, buff);
		}

		/* if anomaly in event j-1,
		   but event j is regular, close the case */

		if (anomaly == 2) {
			printf ("\n");
			fprintf (logfile, "\n");
		}

		if (anomaly & 1) {
			*spurious_count += 1;
			anomaly = 2;
		} else {
			anomaly = 0;
		}
	}

	/* let's check the anomalies count */

	if (count == 0) {
		if (!par->E && anomal_e == par->n) {
			printf ("\
          \n*** It appears to me that all events are anomalous ***\n\
          Please, check the reference time (-t) and the correct\n\
          use of the external clock (-e).\
          Use -E to skip the check.\n");
			return -1;
		}

		delta_i = tm_diff(&(time_list+par->n-1)->timic,
				  &origin->timic) / par->n;
		if (abs(delta_i - par->t) >= par->t/5) {
			printf ("\
          \nIt appears to me that the event rate differs\n\
	  from reference (-t) more than 20%%: %d vs. %d.\n\
	  I will go on, but please, check what you are doing.\n\n",\
				delta_i, par->t);
		}
	}
	return 0;
}

/*
 *     plot time distribution histograms
 */

void plot_histograms (struct params * par,
		      FILE * plot,
		      int * i_differ,
		      int * i_delay,
		      int * e_delay,
		      double * tevi,
		      struct event * time_list) {

	int j, jb, js;
	int side=par->m/par->c + 1.0;
        int size=2*side + 1;
	unsigned int last_channel;
	int bin_size = (par->n + BINS - 1) / BINS;
        float bin_imax[BINS], bin_iavg[BINS];
        float bin_emax[BINS], bin_eavg[BINS];

	if (!par->p) return;
	fprintf (plot, "set multiplot\nclear\n");

	/* i_differ */

	if (par->p & JIT_PM) {
		js = first_non_zero(i_differ, size, &last_channel);
		fprintf (plot,
			 "set size 0.5,0.5\n"
			 "set origin 0.5,0.5\n"
			 "set style data histeps\n"
			 "set log y\n"
			 "set yrange [0.5:*]\n");
		if (par->x) fprintf (plot, "set xrange [%d:%d]\n",
				     -par->x, par->x);
		else fprintf (plot, "set xrange [*:*]\n");
		fprintf (plot, "plot '-' tit 'differences'\n");

		for ( ; js <= last_channel ; js++ ) {
			fprintf (plot, "%f %d\n", (js-side)*par->c*par->g,
				 i_differ[js]);
		}
		fprintf (plot, "e\n");
	}

	/* i_delay and e_delay */

	if (par->p & (DEL_PM | LAT_PM)) {
		fprintf (plot,
			 "set size 0.5,0.5\n"
			 "set origin 0,0.5\n"
			 "set style data histeps\n"
			 "set log y\n"
			 "set yrange [0.5:*]\n");
		if (par->x) fprintf (plot, "set xrange [0:%d]\n", par->x);
		else fprintf (plot, "set xrange [0:*]\n");

		fprintf (plot, "plot %s %s %s",
			 par->p & DEL_PM ? "'-' tit 'delays (clock)'" : "",
			 (par->p & DEL_PM) && (par->p & LAT_PM) ? "," : "", 
			 par->p & LAT_PM ? "'-' tit 'delays (counter)'\n" :
			 "\n");

		if (par->p & DEL_PM) {
			j = js = first_non_zero(i_delay, size, &last_channel);
			for ( ; js <= last_channel ; js++ ) {
				fprintf (plot, "%f %d\n",
					 (js-j)*par->c*par->g, i_delay[js]);
			}
			fprintf (plot, "e\n");
		}
		if (par->p & LAT_PM) {
			js = first_non_zero(e_delay, size, &last_channel);
			for ( ; js <= last_channel ; js++ ) {
				fprintf (plot, "%f %d\n",
					 js*par->c*par->g, e_delay[js]);
			}
			fprintf (plot, "e\n");
		}
	}

	/*
	 *   clock march
	 */

	if (par->p & MRC_PM) {
		for ( js=0 ; js<par->n ; js += bin_size ) {
			jb = js / bin_size;
			bin_imax[jb] = tevi[js];             /* internal max */
			bin_iavg[jb] = tevi[js];             /* internal avg */
			bin_emax[jb] = diff_e(time_list+js); /* external max */
			bin_eavg[jb] = diff_e(time_list+js); /* external avg */

			for ( j=1 ; j<bin_size && j+js < par->n ; j++) {
				if (tevi[j+js] > bin_imax[jb])
					bin_imax[jb] = tevi[j+js];
				if (diff_e(time_list+j+js) > bin_emax[jb])
					bin_emax[jb] = diff_e(time_list+j+js);
				bin_iavg[jb] += tevi[j+js];
				bin_eavg[jb] += diff_e(time_list+j+js);
			}
			bin_iavg[jb] /= bin_size;
			bin_eavg[jb] /= (bin_size/par->c);
			bin_emax[jb] *= par->c;
		}

		fprintf (plot,
			 "set size 1,0.5\n"
			 "set origin 0,0\n"
			 "set nolog y\n"
			 "set style data line\n"
			 "set yrange [*:*]\n"
			 "set xrange [*:*]\n");
		fprintf (plot, "plot %s %s", "'-' tit 'i-max','-' tit 'i-avg'",
			 par->e ?
			 ",'-' tit 'e-max', '-' tit 'e-avg'\n" : "\n");

		for ( js=0 ; js<=jb ; js++ )
			fprintf (plot, "%d %f\n", js*bin_size, bin_imax[js]);
		fprintf  (plot, "e\n");

		for ( js=0 ; js<=jb ; js++ )
			fprintf (plot, "%d %f\n", js*bin_size, bin_iavg[js]);
		fprintf  (plot, "e\n");

		if (par->e) {
			for ( js=0 ; js<=jb ; js++ )
				fprintf (plot, "%d %f\n",
					 js*bin_size, bin_emax[js]);
			fprintf  (plot, "e\n");

			for ( js=0 ; js<=jb ; js++ )
				fprintf (plot, "%d %f\n",
					 js*bin_size, bin_eavg[js]);
			fprintf  (plot, "e\n");
		}
	}
	fprintf  (plot, "unset multiplot\n");
	fflush (plot);
}
