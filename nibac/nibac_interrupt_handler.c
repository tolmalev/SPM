/*
 * nibac_irq_handler
 */

#define MAX_COUNT 10
#define MIN_INTERVAL 25

int nibac_irq_handler (int irq,
		       void * dev,
		       struct pt_regs *regs) {

        u32 lcisr2, lcimr2;
	//  u32 lcisr1;
	//  u32 AI_Status_1;
	int i;
	static int count = MAX_COUNT;
	static struct event_head * data=NULL, * event=NULL;
	static struct timespec timings[2]={{0,0},{0,0}};
	static struct timespec adc_time[2];
	static u32 ping_pong=0;        /* ping-pong buffer parity */
	int16_t *to, *from;

	getnstimeofday(timings);

	lcimr2 = ioread32(mwm[0] + 0x10);
	lcisr2 = ioread32(mwm[0] + 0x14);           /* read interrupt status */
	if (!(lcisr2 & 0x80020000)) return IRQ_NONE;     /* not my interrupt */

	/*
	 *   interrupt overrun protection is triggered by MAX_COUNT interrupts
	 *   spaced  less than MIN_INTERVAL usec each other;
	 *   disable protection only if you **really** know what you are doing
	 */

	if (count-- < 0) {
		iowrite32(0x40000000, mwm[0]+0x10);
		printk (KERN_ALERT
                        "%s:%s - interrupt disabled after overrun\n", HERE);
	}
	if (usec_diff(timings, timings+1) > MIN_INTERVAL)
		count = MAX_COUNT;
  
	/* process the interrupt and store data */

	spin_lock(spm_global.irq_lock);

	data = spm_global.irq->data;    /* always start with fixed buffer */

	data->irq_time.sec = timings[0].tv_sec;
	data->irq_time.nsec = timings[0].tv_nsec;

	data->n_adc = spm_global.irq->sample_adc;
	data->n_dac = spm_global.irq->sample_dac;
	data->samples = spm_global.irq->samples_per_point;
   
	//  lcisr1 = ioread32(mwm[0] + 0xc);

	getnstimeofday (adc_time);               /* fill the data area */

	to = spm_global.irq->adc;

	if (dma_use) {                   /* read through dma */
		
		from = (void *)dma_buffer + ping_pong;
		
		for ( i=0 ; i < data->n_adc * data->samples ; i++ ) {
                        to[i] = from[i];
                }
		iowrite32 (dma_address ^ ping_pong, mwm[0] + 0x528);
		ping_pong ^= 0x800;

	} else {                         /* read through fifo */
                for ( i=0 ; i < data->n_adc ; i++ ) {

                        if (NIBIT(mwr, AI_Status_1, read, AI_FIFO_Empty_St))
                                break;
                        to[i] =  NIREG (mwr, AI_FIFO_Data, read);
                }

                /* check for excess FIFO data */

                if (i == data->n_adc) {
                        for ( ; i<64 ; i++ ) {
                                if (NIBIT(mwr, AI_Status_1, read,
                                          AI_FIFO_Empty_St)) break;
                                NIREG (mwr, AI_FIFO_Data, read);
                        }
                }
	}
	getnstimeofday (adc_time+1);
	data->r_adc = i;

	/* adc_time is stored in tens of nsec */

	data->adc_time = nsec_diff(adc_time+1, adc_time)/10;

	/*
	 *   Execute the code for feedback algorithms. A pointer
	 *   to the buffer where data have been stored is returned.
	 */

	event = ((struct nibac_pci_device *)dev)->daq_ops->spm_irq_handler
		(data);

	/* write DAC's and digital lines for master board */
	
	if (event->n_dac > 0) NIREG(mwr, DAC_Direct_Data, 0, write,
				   IRQ_DAC[0]);
	if (event->n_dac > 1) NIREG(mwr, DAC_Direct_Data, 1, write,
				   IRQ_DAC[1]);
	NIREG(mwr, Static_Digital_Output, write, event->byte[0]);

	/* execute the code for the second write */

	((struct nibac_pci_device *)dev)->daq_ops->second_write(event, 2);

	/* whatever the source, acknoweledge the interrupt */
	
	NIBIT(mwr, Interrupt_A_Ack, write, AI_STOP_Interrupt_Ack, 1);
	iowrite32(0x40, mwm[0] + 0x500);
	
	getnstimeofday(timings+1); /* irq overrun protection - don't touch */

	event->service_time = usec_diff(timings+1, timings);

	spin_unlock(spm_global.irq_lock);

        return (IRQ_HANDLED);
}
