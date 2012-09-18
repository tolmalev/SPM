/*
 *
 *   data structures common to kernel modules and applications
 *
 */

#define HERE NAME, __FUNCTION__
#define MMAP_REGIONS 4            /* max allowed bars */

#if DEBUG == 1
#define dprintk(...) printk (__VA_ARGS__);
#else
#define dprintk(...)    /* suppress this messages */
#endif

/* natively time is measured in nsec */

#define SEC_DIVISOR 1000000000

/*
 *            THE "EVENT"
 *
 *  An event is composed of an head packed
 *  together (i.e. strictly followed) by a
 *  vector containing
 *        n_dac + n_adc * samples
 *  16-bit values (int16_t).
 *
 *  The size of an event is variable, hence
 *  the event data block cannot be arranged
 *  as an ordinary structure, but some macros
 *  make event handling easy.
 *
 *  The event head is a structure of fixed
 *  size. To avoid differences among 32 and
 *  64 bit machines the time format has been
 *  defined as a two field sec+nsec structure
 *  with fixed 32 bit per field, even if this
 *  format is slightly less efficient on 64
 *  bit machines.
 *
 */ 

struct event_head {
	struct irq_time {         /* interrupt service time */
		int32_t nsec;
		int32_t sec;
	} irq_time;
	u_int8_t n_adc, n_dac;    /* vector elements for ADC's and DAC's */
	u_int16_t samples;        /* ADC samples per point */
	u_int16_t adc_time;       /* ADC read time */
	u_int16_t service_time;   /* total sevice time */
	int8_t byte[2];           /* values to digital lines */
	u_int16_t r_adc;          /* actually read ADC values */
};

/*
 *   The following macro definitions can be useful in user space to
 *   handle event data blocks obtained from the 'scan' device.
 *
 *   Occasionally they may be used in kernel space (with caution).
 *
 *   Their use should be definitely avoided in interrupt context, where
 *   proper macro's are available (see feedback/template.c and include/
 *   spm_bac.h). 
 *
 *   'event' is a pointer to an event data block
 *
 *   N_DAC(event)      (int) value of n_dac field
 *   N_ADC(event)      (int) value of n_adc field
 *   SAMPLES(event)    (int) value of samples field
 *   EVENT_SIZE(event) (int) total size (in bytes) of the event data block
 *   DAC(event)        (int16_t *) pointer to the DAC vector
 *   ADC(event)        (int16_t *) pointer to the full ADC vector
 *                                 (n_adc * samples values)
 *   SAMPLE(event,n)   (int16_t *) pointer to the ADC vector of the n-th
 *                                 sample (n_adc values)
 *
 *   'buffer' is a pointer to a vector of event data blocks
 *
 *   J_SLOT(buffer,j)  (void *) pointer to the j-th event data block
 *   J_EVENT(buffer,j) same as J_SLOT, with type (struct event_head *)
 *
 *
 *   The SLOT_SIZE(adc,dac,samples) macro can be used to foresee the
 *   event data block size from its parameters: number of ADC and DAC
 *   channels and sampling repetion.
 *
 */

#define HEAD_SIZE (sizeof (struct event_head))

#define EVENT_HEAD(buffer) ((struct event_head *)(buffer))

#define N_DAC(event) (EVENT_HEAD(event)->n_dac)
#define N_ADC(event) (EVENT_HEAD(event)->n_adc)
#define SAMPLES(event) (EVENT_HEAD(event)->samples)
#define EVENT_SIZE(event) (HEAD_SIZE + sizeof(int16_t) *		\
		          (N_DAC(event) + N_ADC(event) * SAMPLES(event)))

#define DAC(event) ((int16_t *)((void *)(event) + HEAD_SIZE))
#define ADC(event) (DAC(event) + N_DAC(event))
#define SAMPLE(event,n) (ADC(event) + (n) * N_ADC(event))

#define J_SLOT(buffer,j) ((void *)(buffer) + (j) * EVENT_SIZE(buffer))
#define J_EVENT(buffer,j) ((struct event_head *) J_SLOT(buffer,j))

#define SLOT_SIZE(adc,dac,samples) (HEAD_SIZE + sizeof(int16_t) *       \
                                    ((dac) + (adc) * (samples)))

/*
 * The parameter structure to be used for a scan request
 *
 */

struct scan_params {
	char code[2];
	u16 mask;

	u32 lines_per_frame;
	u32 points_per_line;
	u32 cadence_usec;
	u32 buffer_size;
	u32 buffer_size_min;
	u32 high_water;
	u32 timeout;
        u32 samples_per_point;
        u32 sample_adc;
        u32 sample_dac;
};

/* Flags for the mask in scan_params */

#define LINES_PER_FRAME   0x0001
#define POINTS_PER_LINE   0x0002
#define CADENCE_USEC      0x0004
#define BUFFER_SIZE       0x0008
#define BUFFER_SIZE_MIN   0x0010
#define HIGH_WATER        0x0020
#define TIMEOUT           0x0040
#define SAMPLES_PER_POINT 0x0080
#define SAMPLE_ADC        0x0100
#define SAMPLE_DAC        0x0200
#define PAY_LOAD          0x0400


/*
 *   info request
 */

struct info_request {
        struct {
                u_int32_t lines_per_frame;
                u_int32_t points_per_line;
                u_int32_t cadence_usec;
                u_int32_t slot_size;
                u_int32_t buffer_size;
                u_int32_t samples_per_point;
                u_int32_t sample_adc;
                u_int32_t sample_dac;
        } now, new;
};
