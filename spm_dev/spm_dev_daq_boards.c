/*
 *     register a DAQ board
 */

struct spm_external_functions * register_daq_board (
        int is_master,
        void * set_timer_or_2nd_write,
        void * set_samples,
        void * set_adc) {

	unsigned long spin_temp;
	struct spm_external_functions * status = &daq_ops;

	if (is_master) {                     /* board registers as master */

		spin_lock_irqsave (&irq_lock, spin_temp);
		if (daq_ops.spm_irq_handler == (void *) null_function) {
			if (set_timer_or_2nd_write)
                                daq_ops.set_timer = set_timer_or_2nd_write;
                        if (set_samples) daq_ops.set_samples = set_samples;
                        if (set_adc) daq_ops.set_adc = set_adc;
                        daq_ops.spm_irq_handler = spm_irq_handler;
		} else {
			status = NULL;
		}
		spin_unlock_irqrestore (&irq_lock, spin_temp);

	} else {                             /* board registers as slave */

		spin_lock_irqsave (&irq_lock, spin_temp);
		if (daq_ops.second_write == (void *) null_function &&
                    set_timer_or_2nd_write) {
			daq_ops.second_write = set_timer_or_2nd_write;
		} else {
			status = NULL;
		}
		spin_unlock_irqrestore (&irq_lock, spin_temp);
        }

        printk (KERN_ALERT "%s:%s - Register %s status: %p\n", HERE,
                is_master ? "master" : "slave", status);

	return status;
}

/*
 *     unregister a DAQ board
 */

void unregister_daq_board (int is_master) {

	unsigned long spin_temp;

	printk (KERN_ALERT "%s:%s - releasing %s board\n", HERE,
		is_master ? "master" : "slave");
  
	if (is_master) {                       /* release master board */

		spin_lock_irqsave (&irq_lock, spin_temp);

		daq_ops.set_timer = (void *) null_function;
                daq_ops.set_samples = (void *) null_function;
                daq_ops.set_adc = (void *) null_function;
                daq_ops.spm_irq_handler = (void *) null_function;

		spin_unlock_irqrestore (&irq_lock, spin_temp);

	} else {

		spin_lock_irqsave (&irq_lock, spin_temp);
		daq_ops.second_write = (void *) null_function;
		spin_unlock_irqrestore (&irq_lock, spin_temp);
	}
}

/*
 *    null function: immediately return
 */

void null_function (void) { };

/*
 *    spm_irq_handler
 */

struct event_head * spm_irq_handler (struct event_head * irq_buf) {

	struct event_head * data;

	/* process the interrupt and store data */

	if (irq.action == Go_Process) {
		if (irq.buffer_count == irq.buffer_size) {
			printk (KERN_ALERT "%s:%s - irq overrun with %d "
				"lines and %d points pending\n",
				HERE, irq.lines, irq.points);
			irq.action = Idle;
			data = irq_buf;
			irq.completion = Irq_Overrun;
		} else {
			data = (void *) irq.unibuf +
					irq.write_pointer * irq.slot_size;
		}

	} else {
		data = irq_buf;
	}

 	if (irq_buf->r_adc != irq_buf->n_adc * irq_buf->samples)
		printk (KERN_ALERT "%s:%s - FIFO ERROR: r:%d n:%d s:%d"
                        "act:%d  l:%d  p:%d  t:%d\n", HERE, irq_buf->r_adc,
                        irq_buf->n_adc, irq_buf->samples,
			irq.action, irq.lines, irq.points, irq_buf->adc_time);

	user_feedback_code(&irq);    /* go execute user algorithms */

	/* if active, transfer DAC data and update counters */

	if (irq.action == Go_Process) {

		irq.sync = 0;
    
		++irq.buffer_count;
		if (++irq.write_pointer == irq.buffer_size)
			irq.write_pointer = 0;
   
		if (--irq.points <= 0) {

			dprintk (KERN_ALERT "%s:%s - eol %d - buffer: %d\n",
				 HERE, irq.lines, irq.buffer_count);

			if (irq.lines && --irq.lines <= 0) {
				dprintk (KERN_ALERT
					 "%s%s - end of frame\n", HERE);
				irq.action = Idle;
				irq.awake_in = 1;
				irq.completion = Irq_None;
			} else {
				irq.points = irq.points_per_line;
			}
		}

		if (irq.request == Go_Idle || irq_stop_request == 'A') {
			irq.action = Idle;
			irq.awake_in = 1;
			irq_stop_request = 0;
			irq.request = Again;
			irq.completion = Irq_Stopped;
		}

		if (irq.awake_in && --irq.awake_in <= 0) {
			ready_to_read = 1;
			wake_up_interruptible(&scan_wait_queue);
		}
	}

        if (data != irq_buf) memcpy (data, irq_buf, irq.slot_size);

	return data;
}

/*
 *     register/unregister an user feedback routine
 */

int register_feedback_code (void * feedback_code) {

	unsigned long spin_temp;
	
	if (user_feedback_code != (void *)null_function) {
		return (-1);
	}
	
	spin_lock_irqsave (&irq_lock, spin_temp);
	user_feedback_code = feedback_code;
	spin_unlock_irqrestore (&irq_lock, spin_temp);
	
	return (0);

}

void unregister_feedback_code (void * feedback_code) {

        unsigned long spin_temp;

	spin_lock_irqsave (&irq_lock, spin_temp);
	user_feedback_code = (void *)null_function;
	spin_unlock_irqrestore (&irq_lock, spin_temp);
}
