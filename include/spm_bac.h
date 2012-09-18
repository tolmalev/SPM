/*
 *
 *   data structures common to all kernel stuff
 *
 */

/*
 * MAX_BOARDS is the number of DAQ boards the driver can support;
 *          the first board is the master;
 *          the second board is the slave.
 */

#define MAX_BOARDS 2

/*
 *   usec_diff : takes two timespec (unix time in sec and NANOsec) and
 *               computes time difference (in MICROsec)
 */

inline int usec_diff (struct timespec * a, struct timespec * b) {
	return ((a->tv_sec - b->tv_sec)*1000000 +
		(a->tv_nsec - b->tv_nsec)/1000);
}

inline int nsec_diff (struct timespec * a, struct timespec * b) {
	return ((a->tv_sec - b->tv_sec)*1000000000 +
		(a->tv_nsec - b->tv_nsec));
}

/*
 *   All parameters of interest in the interrupt context.
 *   These fields contain the  **actual** values of the
 *   parameters, hence are to be modified with proper care
 *   and synchronization.
 */

struct irq_parameters {

        struct event_head * unibuf;  /* main buffer for event storage */
        enum {Idle, Go_Process} action;
        int cadence_usec;
        enum {Irq_None, Irq_Overrun, Irq_Stopped} completion;
        enum {Again, Go_Idle} request;

        int allow_new_slot;

	int byte_size;          /* total buffer size (in bytes) */
        int buffer_size;        /* total buffer size (in slots) */
        int write_pointer;      /* in slots */
        int read_pointer;       /* in bytes */
        int awake_in;
        int buffer_count;       /* slots (or part of a slot) in buffer */
        int slot_size;          /* slot size in bytes */

	int sample_adc;         /* ADC's reading in a sample */
        int sample_dac;         /* DAC's write in a sample */
        int samples_per_point;  /* repetition of ADC reading in an event */

	struct event_head * data;    /* auxiliary buffer for current event */
	int16_t * adc;          /* the ADC data vector */
	int16_t * dac;          /* the DAC data vector */

        int points;             /* points in current line to be processed */
        int lines;              /* lines to be processed */
        int points_per_line;    /* self explaining */
        int lines_per_frame;    /* self explaining */
        int sync;

        int pay_load_size;  /* size (bytes) of free vector area 'pay_load' */
        void * pay_load;    /* free area for vector data exchange */
};

#define IRQ_DAC (spm_global.irq->dac)

/*
 *   Macro's for event handling while in interrupt context.
 *   While in the Go_Process state the macro's retrieve the previous n-th
 *   event in the buffer. The buffer is always guaranteed to contain the
 *   current line and at least a full previous line. In the Idle state,
 *   the macro's always return the current event.
 *
 *   EVENT(n) pointer to the 'struct events_head' data structure of the
 *            previous n-th event.
 *
 *   EVENT_DAC(n) pointer to the DAC data vector of previous n-th event.
 *   EVENT_ADC(n) pointer to the ADC data vector of previous n-th event.
 *
 *   EVENT_SAMPLE(n,m) pointer to the ADC data vector of sample m of
 *                     previous n-th event.
 *
 *   EVENT(0) is the current event, n < 0 is converted always to the
 *   current event. 
 */

#define EVENT_OFFSET(n) (irq->write_pointer - (n) < 0 ?			\
			 (irq->write_pointer+irq->buffer_size-(n)) :	\
			 (irq->write_pointer - (n)))
#define EVENT_BUFFER(n) ((void *) irq->unibuf + \
			 EVENT_OFFSET(n) * irq->slot_size)

#define EVENT(n) ((irq->action != Go_Process || n <= 0) ?	\
		  (irq->data) : (struct event_head *) EVENT_BUFFER(n))

#define EVENT_DAC(n) ((int16_t *)((void *) (EVENT(n)) + HEAD_SIZE))
#define EVENT_ADC(n) (EVENT_DAC(n) + irq->sample_dac)
#define EVENT_SAMPLE(n,m) (EVENT_ADC(n) + (m) * EVENT(n)->n_adc)

/*
 *  Prototype of the user-supplied function
 *  to implement the feedback algorithm.
 */

void feedback_code (struct irq_parameters *);

/*
 *    Dispatch table for the interrupt handler
 *
 *    Interrupt handling is completed in several steps:
 *
 *    1) the interrupt handler registered for the master board is
 *       executed by the kernel;
 *    2) the interrupt handler performs housekeeping tasks, acknoweledges
 *       the interrupt and reads timer and ADCs.
 *    3) the interrupt handler enters the spm_irq_handler function in
 *       the dispatch table;
 *    4) the spm_irq_handler function takes care of scan pointers, then
 *       calls the user feedback_code function;
 *    5) on return of the user feedback_code function, the spm_irq_handler
 *       exits and values to master board DACs are written;
 *    6) the second_write() function in the dispatch table is executed
 *       to write data to slave board DACs;
 *    7) the board interrupt handler exits after completing irq servicing;
 *
 *    feedback_code() is a function in the spm_dev module and includes
 *                    the user code with feedback algorithms
 *    second_write()  is a function in the slave board driver; it is
 *                    registered during slave board initialization by a
 *                    call to spm_register_daq_board()
 *    set_timer()     is a service function in the master board driver;
 *                    it is registered during the master board
 *                    initialization by a call to spm_register_daq_board()
 *
 */

struct spm_external_functions {
	struct event_head * (* spm_irq_handler) (struct event_head *);
	void (* second_write) (struct event_head *, int);
	void (* set_timer) (int);
	void (* set_samples) (int);
	void (* set_adc) (int);
};

/*
 *  Any global common to any of the spm components.
 */

struct spm_export_symbols {
	struct spm_external_functions * (* register_daq_board) (
                int , void *, void *, void *);
	void (* unregister_daq_board) (int);
	int (* get_devnumber) (void);
        void (* release_devnumber) (int);
	spinlock_t * irq_lock;
	struct irq_parameters * irq;
        int (* register_feedback_code) (void *);
        void (* unregister_feedback_code) (void *);
};

/* common services */

/*
 *     Obtain a device lock.
 *     If the request is O_NONBLOCK and the lock cannot
 *     be obtained return -EBUSY.
 *     Otherwise obtain the lock or sleep.
 */

#define GET_THE_LOCK(file,lock) \
        if (file->f_flags & O_NONBLOCK) { \
                if (down_trylock(&lock)) { \
                        return -EBUSY; \
                } \
        } else if (down_interruptible (&lock)) { \
                printk (KERN_WARNING "%s:%s - request for %i interrupted\n", \
                        HERE, current->pid); \
                return -EINTR;\
        }

/*
 *    Send a log message both to syslog and to current tty
 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define DOUBLE_LOG(format,...) \
{ \
        char buf[80]; \
        snprintf (buf, 80, format, __VA_ARGS__); \
        printk (KERN_ALERT "%s", buf); \
        if (current->signal->tty) { \
                current->signal->tty->driver->write ( \
                        current->signal->tty, buf, strlen(buf)); \
                current->signal->tty->driver->write ( \
                        current->signal->tty, "\r", 1); \
         } \
}                       
#else
#define DOUBLE_LOG(format,...) \
{ \
        char buf[80]; \
        snprintf (buf, 80, format, __VA_ARGS__); \
        printk (KERN_ALERT "%s", buf); \
        if (current->signal->tty) { \
                current->signal->tty->driver->ops->write ( \
                        current->signal->tty, buf, strlen(buf)); \
                current->signal->tty->driver->ops->write ( \
                        current->signal->tty, "\r", 1); \
        } \
}
#endif

