

struct event {
	struct timespec timic;  /* event time on the internal clock */
	int timec;              /* latency on the external clock */
};

#define SLOT sizeof(struct event)
#define PAD (SLOT-1)
