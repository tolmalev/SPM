
#define display \
"Usage: frame [OPTIONS]\n"\
"\n"\
" -a   <int> minimum buffer size (slots)\n"\
" -b   <int> buffer size (slots)\n"\
" -c   <int> cadence (usec)\n"\
" -h   this help\n"\
" -l   <int> lines per frame\n"\
" -p   <int> points per line\n"\
" -t   <int> timeout (msec?)\n"\
" -v   debug\n"\
" -w   <int> high water level\n"\
"\n"\
" -d   compute and plot distribution of interrupt service times\n"\
" -D   <string> command for distribution time analysis\n"\
" -F   show frame\n"\
" -C   <int> ADC channel for image [0]\n"\
" -R   rescale data according to calibration table\n"\
" -L   <int> lines to be plotted (0 ... 5) [3]\n"\
" -S   save frame data to file\n"


struct params {
	int a,b,c,l,p,t,v,w,d;
	char * D;
	int F,C,R,L,S;
};

#define PAR_DEFAULT {0,0,0,-1,-1,0,0,0,0,NULL,0,0,0,3,0}
/*                   a b c  l  p t v w d   D  F C R L S    */
#define PAR_NAMES "a:b:c:hl:p:m:t:vw:dD:FC:RL:S"
#define BUFLEN 512
#define PARAMS_DIR "/sys/module/spm_dev/parameters/"

#define IVENT(buf,i) ((struct event_head *) ((void *)(buf) + (i) * slot_size))

struct event_times {
	struct irq_time irq_time;   /* interrupt time on the internal clock */
	int timec;                  /* latency time on the external clock */
};

struct statistics {
	int fifo_errors;
	struct terna {
		double min;
		double avg;
		double max;
	} adc, service, interval;
};

char * new_code ();
void catch_the_signal (int);
long int usec(struct timeval *, struct timeval *);
void parse_options (int , char **, struct params *);
int read_parameter (char *);
double usec_diff (struct irq_time *, struct irq_time *);
int get_a_line (u8 * , int, int);
void statistics (struct event_head *, struct statistics *, int, int);
void create_image (struct event_head *, struct scan_params *,
		   int, int, int, int, int);
void line_plot (FILE *, int, struct params *, int, struct event_head *,
                struct scan_params *, int samples_per_point);
