
#define display \
"\nread - fill a buffer with interrupt timings\n"\
"\n"\
"Usage: read [OPTION]\n"\
"\n"\
" -a   analyze buffers and log spurious values\n"\
" -B   insert block separation into list file\n"\
" -c   <float> resolution / external clock period (usec) [0.25]\n"\
" -d   activate verbose debug (more d's more debug)\n"\
" -e   use external clock values\n"\
" -E   force -e\n"\
" -f   <file> define a log file name instead of timestamp\n"\
" -g   <int> group together spectra bins\n"\
" -h   print this help and exit\n"\
" -i   <string> input device [/dev/latim]\n"\
" -l   list all values\n"\
" -L   list all values into a dedicated file\n"\
" -m   <float> bad time threshold (usec) [120]\n"\
" -n   <int> events to be collected for each buffer [1]\n"\
" -N   <int> buffers to be filled [1]\n"\
" -p   times plot (1:jitter + 2:latency + 4:delay + 8:clock march) [15]\n"\
" -s   report data after every buffer\n"\
" -S   give only final results\n"\
" -t   <int> reference time interval (usec) [128]\n"\
" -U   use a unique big buffer to be saved only on exit\n"\
" -v   debug (more v's more debug)\n"\
" -x   <string> width of spectrum plot X axis\n"

struct params {
	int a,B;
	float c;
	int e,E;
	char * f;
	int g,h;
	char *i;
	int l,L;
	float m;
	int n,N,p,s,S,t,U,v,x;
};

#define PAR_DEFAULT {0,0,0.25,0,0, /*a,B,c,e,E*/ \
   NULL, /*f*/ \
   1,0, /*g,h*/ \
   NULL, /*i*/ \
   0,0, /*l,L*/ \
   120.0, /*m*/ \
   1,1,15, /* n,N,p*/ \
   0,0, /*s,S*/ \
   200,0,0,0 /*t,U,v,x*/ }

#define PAR_NAMES "aBc:deEf:g:hi:lLm:n:N:p:qrRsSt:Uvx:"

/* compute the external counter progress after interrupt request;
   the interrupt requests are generated every CYCLE counts */

#define CYCLE 512
#define diff_e(ev) \
        (((ev)->timec - (((ev) - 1)->timec & ~(CYCLE-1)) - CYCLE) & 0xfff)

/* plot selection masks and other stuff */

#define JIT_PM 1
#define LAT_PM 2
#define DEL_PM 4
#define MRC_PM 8

#define BUFLEN 512
#define BINS 500

void list_events (struct params *, FILE *, struct event *,
                  int, struct event * origin);
int check_events (struct params *, struct event *, FILE *, int, int *,
                  int, struct event *);
void plot_histograms (struct params *, FILE *,int *, int *, int *,
		      double *, struct event *);
