/*
 *
 *    scan_read - read a data buffer
 *
 */

ssize_t scan_read  (struct file * filp,
                    char __user * buf,
                    size_t byte_count,
                    loff_t * offset) {

	u64 jft=0, jfti;
	int result, got_events;
	int not_copied;
	int ret_val=byte_count;
	int up_slot, down_slot;
	unsigned long spin_temp;
	int byte_stored, byte_needed;
	int start_slot, free_slots;
	int high_water_count;
	int byte_in_buffer, byte_transfer;
	int buffer_count, old_buffer_count;
	int go_to_sleep;
	int tmo = timeout * HZ / 1000;
	int current_action;
	int completion;

	dprintk (KERN_ALERT "%s:%s - %d bytes request from device: %d:%d\n",
		 HERE, (int)byte_count,
		 MAJOR(filp->f_dentry->d_inode->i_rdev),
		 MINOR(filp->f_dentry->d_inode->i_rdev));

	/* serializes multiple accesses to the device */

	GET_THE_LOCK(filp,spm_device_in_use);

	/* the driver has not been armed for any operation */

	if (spm_device_action == Nop) {
		DOUBLE_LOG ("%s:%s - No pending operation\n", HERE);
		up (&spm_device_in_use);
		return -EPERM;
	}

	/* operation has been initialized - really start the work */

	if (spm_device_action == Ready) {
		dprintk (KERN_ALERT "%s:%s - starting a read loop\n", HERE);

		spm_device_action = Working;
		ready_to_read = 0;
		got_events = 0;
  
		spin_lock_irqsave (&irq_lock, spin_temp);
		if ((current_action = irq.action) == Idle) {
			irq.action = Go_Process;
			irq.completion = Irq_None;
		}
		spin_unlock_irqrestore (&irq_lock, spin_temp);

		if (current_action != Idle) {
			DOUBLE_LOG ("%s:%s - The loop was not Idle "
			       "as it should!\n", HERE);
			ret_val = -EBUSY;
			goto read_return;
		}
	}

	dprintk(KERN_ALERT
		"%s:%s requested %d bytes at line %d and point %d\n",
		HERE, (int)byte_count, irq.lines, irq.points);

	byte_stored = 0;

	while (1) {

		/* if some data in buffer, move it to user space */

		spin_lock_irqsave (&irq_lock, spin_temp);
		buffer_count = irq.buffer_count;
		current_action = irq.action;
		completion = irq.completion;
		spin_unlock_irqrestore (&irq_lock, spin_temp);

		byte_needed = byte_count - byte_stored;
		byte_in_buffer = buffer_count * SLOT - irq.read_pointer % SLOT;
		if (byte_in_buffer < 0) byte_in_buffer +=
						irq.buffer_size * SLOT;

		if (byte_in_buffer) {
			byte_transfer =	byte_in_buffer > byte_needed ?
				byte_needed : byte_in_buffer;

			/* how many bytes up and down the circular buffer? */

			up_slot = irq.buffer_size * SLOT - irq.read_pointer;
			if (up_slot > byte_transfer) {
				up_slot = byte_transfer;
				down_slot = 0;
			} else {
				down_slot = byte_transfer - up_slot;
			}

			/* copy to user buffer */

			not_copied = copy_to_user(
				buf + byte_stored, (u8 *)irq.unibuf +
				irq.read_pointer, up_slot);

			if (!not_copied && down_slot)
				not_copied += copy_to_user(
					buf + byte_stored + up_slot,
					irq.unibuf, down_slot);

			if (not_copied) {
				DOUBLE_LOG ("%s:%s Error while "
					"copying to user space\n", HERE);
				ret_val = -EFAULT;
				break;
			}

			/* now let's update all pointers */

			byte_stored += byte_transfer;

			start_slot = irq.read_pointer / SLOT;
			irq.read_pointer += byte_transfer;
			free_slots = irq.read_pointer / SLOT - start_slot;

			if (irq.read_pointer >= irq.buffer_size * SLOT)
				irq.read_pointer -= irq.buffer_size * SLOT;

			/* atomically update irq.buffer_count */

			spin_lock_irqsave (&irq_lock, spin_temp);
			irq.buffer_count -= free_slots;
			spin_unlock_irqrestore (&irq_lock, spin_temp);

			/* if we have a valid full buffer,
			   return it to user space */

			if (byte_count == byte_stored) {
				ret_val = byte_count;
				break;
			}
		}

		/* 
		 * More data are needed, but it is useless to sleep if the
		 * loop is Idle. If any more data is in buffer, return it
		 * to user space and handle the completion on next call.
		 */

		if (current_action == Idle) {
			if (byte_stored) {
				up (&spm_device_in_use);
				return byte_stored;
			}

			/* the loop is Idle and no more data are available */

			if (completion == Irq_None) { /* how did it happen? */
				ret_val = 0;
				dprintk (KERN_ALERT "%s:%s - Nop & Irq_None "
					 "-> EOF - buffer: %d\n",
                                        HERE, buffer_count);
			} else if (completion == Irq_Overrun) {
				ret_val = -EIO;
				DOUBLE_LOG ("%s:%s - Nop & Irq_Overrun "
					"-> -EIO\n", HERE);
			} else if (completion == Irq_Stopped) {
				ret_val = -ECANCELED;
				DOUBLE_LOG ("%s:%s - Nop & Irq_Stopped "
					"-> -ECANCELED\n", HERE);
			}
			goto read_return;
		}

		/*
                 *    more data are needed, but we must return now
		 *    if the request was O_NONBLOCK
                 */

		if (filp->f_flags & O_NONBLOCK) {
                        up (&spm_device_in_use);
                        if (byte_stored) return byte_stored;
                        return -EAGAIN;
                }

		high_water_count = (high_water * irq.buffer_size) / 100;
		if (high_water_count < 1) high_water_count = 1;

		spin_lock_irqsave (&irq_lock, spin_temp);
		{
			ready_to_read = 0;
			current_action = irq.action;
			byte_in_buffer = irq.buffer_count * SLOT -
				irq.read_pointer % SLOT;
			if (byte_in_buffer < 0) byte_in_buffer +=
							irq.buffer_size * SLOT;
			byte_needed = byte_count - byte_stored -
				byte_in_buffer;
			if (byte_needed > 0) {
				irq.awake_in = (byte_needed + PAD) / SLOT;

				if (irq.awake_in + irq.buffer_count >
				    high_water_count)
					irq.awake_in = high_water_count -
						irq.buffer_count;
			} else {
				irq.awake_in = 0;
			}
			go_to_sleep = irq.awake_in;
			old_buffer_count = irq.buffer_count;
		}
		spin_unlock_irqrestore (&irq_lock, spin_temp);

		dprintk (KERN_ALERT "%s:%s - going to sleep for %d events\n",
			 HERE, irq.awake_in);

		if (go_to_sleep) {
			jfti = get_jiffies_64();
			result = wait_event_interruptible_timeout (
				scan_wait_queue, ready_to_read!=0, tmo);
			jft = get_jiffies_64() - jfti;
		} else {
			continue;   /* data already in buffer; process them */
		}

		/* check if we received a ctrl^C or a similar signal;
		   put the interrupt loop Idle and clear spm_device_in_use */

		if (ready_to_read == 0 && result != 0) {
			DOUBLE_LOG ("%s:%s - interrupt signal\n", HERE);
			ret_val = -EINTR;
			goto read_return;
		}

		if (ready_to_read == 0 && result == 0) {         /* timeout */
			spin_lock_irqsave (&irq_lock, spin_temp);
			buffer_count = irq.buffer_count;
			spin_unlock_irqrestore (&irq_lock, spin_temp);

			/* give another chance unless there was no progress */

			if (buffer_count == old_buffer_count) {  /* timeout */
				dprintk(KERN_ALERT "%s:%s - timeout\n", HERE);
				ret_val = -ETIME;
				goto read_return;
			}
		}
	}

 read_return:
	if (ret_val <= 0) {                     /* handle error condition */
		spin_lock_irqsave (&irq_lock, spin_temp);
		irq.action = Idle;
		spin_unlock_irqrestore (&irq_lock, spin_temp);
		spm_device_action = Nop;

#if HOLD_BUFFER == 0
		if (irq.unibuf) vfree (irq.unibuf);
		irq.unibuf = NULL;
#endif
	}
	up (&spm_device_in_use);
	return ret_val;
}

/*
 *
 *    scan_write - write function
 *
 *    this call is used `to write' command and parameters
 *    to the device
 *
 */

ssize_t scan_write (struct file * filp,
		    const char __user * user_buf,
		    size_t count,
		    loff_t * offset) {

	int not_copied;
	int ret_val=count;
	int i;
	unsigned long spin_temp;
	int small_buffer;
        int new_samples, new_adc, new_dac, new_size;
        struct event_head * new_data;

	struct scan_params command;

	dprintk (KERN_ALERT
		 "%s:%s - write request with %d bytes from %i\n",
		 HERE, (int) count, current->pid);

	if (filp->f_flags & O_NONBLOCK) {
		if (down_trylock(&spm_device_in_use)) {
			return -EBUSY;
		}
	} else if (down_interruptible (&spm_device_in_use)) {
		DOUBLE_LOG ("%s:%s - request for %i interrupted\n",
			HERE, current->pid);
		return -EINTR;
	}

	/* get the command from user space */

	if (count == 1 || count == 2) {
                not_copied = copy_from_user(&command, user_buf, count);
        } else if (count >= sizeof(command)) {
                not_copied =
                        copy_from_user(&command, user_buf, sizeof(command));
        } else {
                DOUBLE_LOG ("%s:%s - invalid command size: %d\n",
			    HERE, (int) count);
		ret_val = -EINVAL;
		goto write_return;
	}

        /* check the command code */

	if (command.code[0] == 'C') {
		DOUBLE_LOG ("%s:%s - aborting current operation ...\n",
			HERE);
    
		spin_lock_irqsave (&irq_lock, spin_temp);
		irq.action = Idle;
		spin_unlock_irqrestore (&irq_lock, spin_temp);

		spm_device_action = Nop;

#if HOLD_BUFFER == 0
		if (irq.unibuf) vfree (irq.unibuf);
		irq.unibuf = NULL;
#endif

		goto write_return;
	}

	if (command.code[0] != 's' && command.code[0] != ' ' &&
            command.code[0] != 'p') {
		DOUBLE_LOG ("%s:%s - invalid request code: %d\n",
			HERE, command.code[0]);
		ret_val = -EINVAL;
		goto write_return;
	}

        if (spm_device_action != Nop) {
	        DOUBLE_LOG ("%s:%s - operation in progress\n", HERE);
	        ret_val = -EBUSY;
	        goto write_return;
        }
  
        if (irq.action != Idle) {
	        DOUBLE_LOG ("%s:%s - You have a problem: the loop "
		        "is not Idle\n", HERE);
	        ret_val = -EBUSY;
	        goto write_return;
        }

        /* we have a valid command in a valid context - let's go on
           and get the command parameters and 'pay_load' area */

        if (count >= sizeof(command)) {

                dprintk (KERN_ALERT "%s:%s - command mask for scan: %x\n",
			 HERE, command.mask);

		if (command.mask & LINES_PER_FRAME) {
			lines_per_frame = command.lines_per_frame;
		}
		if (command.mask & POINTS_PER_LINE) {
			points_per_line = command.points_per_line;
		}
		if (command.mask & CADENCE_USEC) {
			cadence_usec = command.cadence_usec;
		}
		if (command.mask & BUFFER_SIZE) {
			buffer_size = command.buffer_size;
		}
		if (command.mask & BUFFER_SIZE_MIN) {
			buffer_size_min = command.buffer_size_min;
		}
		if (command.mask & HIGH_WATER) {
			high_water= command.high_water;
		}
		if (command.mask & TIMEOUT) {
			timeout = command.timeout;
                }
		if (command.mask & SAMPLES_PER_POINT) {
			samples_per_point = command.samples_per_point;
                }
		if (command.mask & SAMPLE_ADC) {
			sample_adc = command.sample_adc;
                }
		if (command.mask & SAMPLE_DAC) {
			sample_dac = command.sample_dac;
                }
                if (command.mask & PAY_LOAD) {
                        if (pay_load) vfree(pay_load);

                        pay_load_size = count - sizeof(command);
                        if (pay_load_size) {
                                pay_load = vmalloc(pay_load_size);

                                if (!pay_load) {
                                        DOUBLE_LOG ("%s:%s - %s\n", HERE,
                                                    "could not allocate memory"
                                                    "for PAY_LOAD");
                                        ret_val = -EINVAL;
                                        goto write_return;
                                }

                                not_copied = copy_from_user
                                        (pay_load, user_buf + sizeof(command),
                                         pay_load_size);

                                if (not_copied) {
                                        vfree(pay_load);
                                        pay_load = NULL;
                                        ret_val = -EFAULT;
                	                goto write_return;
                                }
                        }
                }
        }

        /*  if only parameter setup has been requested, return now  */

        if (command.code[0] == ' ') goto write_return;

        /*
         *  If samples_per point or sample_adc or sample_dac have been
         *  specified try to allocate a new slot. Otherwise, give up.
         */

        new_samples = samples_per_point;
        new_adc = sample_adc;
        new_dac = sample_dac;

        if (new_samples != irq.samples_per_point ||
            new_adc != irq.sample_adc || new_dac != irq.sample_dac) {
                if (!irq.allow_new_slot &&
		    (new_samples != irq.samples_per_point ||
		     new_adc != irq.sample_adc)) {
                        DOUBLE_LOG ("%s:%s - %s\n", HERE,
                                    "New slot on-the-fly not allowed");
                
                        samples_per_point = irq.samples_per_point;
                        sample_adc = irq.sample_adc;
                        sample_dac = irq.sample_dac;
                        ret_val = -EINVAL;
                        goto write_return;
                }

                new_size = sizeof (struct event_head) + 2 *
                        (new_adc * new_samples + new_dac);
                new_data = kmalloc(new_size, GFP_KERNEL);
                if (!new_data) {
                        printk (KERN_ALERT "%s:%s - %s\n", HERE,
                                "Allocation for new slot failed.\n");
                        ret_val = -ENOMEM;
                        goto write_return;
                }

                /* now synchronize with the interrupt handler */

                spin_lock_irqsave (&irq_lock, spin_temp);

                kfree (irq.data);
                irq.data = new_data;
                irq.slot_size = new_size;
                irq.samples_per_point = new_samples;
                irq.sample_adc = new_adc;
                irq.sample_dac = new_dac;
                irq.dac = (void *) irq.data + sizeof (struct event_head);
                irq.adc = irq.dac + irq.sample_dac;

                (* daq_ops.set_samples) (irq.samples_per_point);
                (* daq_ops.set_adc) (irq.sample_adc);

                spin_unlock_irqrestore (&irq_lock, spin_temp);
                slot = irq.slot_size;
        }

        /*
         *    set up all required data and arm the scan machine - it should
         *    be safe to work with all irq resources, ***apart irq.action***
         */

        irq.points_per_line = points_per_line;
        irq.lines_per_frame = lines_per_frame;
        irq.cadence_usec = cadence_usec;
        irq.sync = 1;
        irq.pay_load = pay_load;
        irq.pay_load_size = pay_load_size;

        /*
         *   lines_per_frame == 0   means endeless loop;
         *   points_per_line must be > 0 even if not used;
         *   100 usec <= irq_cadence <= 1 sec
         */

        if (irq.points_per_line <= 0 || irq.lines_per_frame < 0 ||
            irq.cadence_usec * irq.samples_per_point < 100 ||
            irq.cadence_usec * irq.samples_per_point > 2000000 ||
            irq.cadence_usec < 5 * irq.sample_adc) {
	        ret_val = -EINVAL;
	        DOUBLE_LOG ("%s:%s - Invalid scan parameters!\n", HERE);
                ret_val = -EINVAL;
	        goto write_return;
        }

        irq.write_pointer = 0;
        irq.read_pointer = 0;
        irq.points = irq.points_per_line;
        irq.lines = irq.lines_per_frame;
        irq.awake_in = irq.points_per_line;
        irq.buffer_count = 0;

        /*  Allocate buffer memory 
         *
         *  'buffer_size' is the proposed size in 'slot'
         *  (1 slot == 1 event).
         *  The actual buffer can be smaller, if memory is scarce,
         *  but must be at least two lines or buffer_size_min,
         *  whichever is greater.
         */

        if (buffer_size_min < MINIMAL_BUF ||
                        high_water < 0 || high_water > 100) {
	        DOUBLE_LOG ("%s:%s - Invalid buffer parameters!\n", HERE);
                ret_val = -EINVAL;
	        goto write_return;
        }

        small_buffer = 4 * irq.points_per_line;
        if (buffer_size_min > small_buffer) small_buffer = buffer_size_min;
        if ((i = buffer_size) < small_buffer) i = small_buffer;

        /* free old buffer if cannot reuse and allocate new one */

        if (irq.unibuf) {
		if (i * irq.slot_size != irq.byte_size) {
			vfree (irq.unibuf);
			irq.unibuf = NULL;
		}
        }

	if (!irq.unibuf) {
		for ( ; i>=small_buffer ; i/=2) {
			dprintk (KERN_ALERT "%s:%s - trying vmalloc of %d "
				 "bytes for %d slots\n", HERE, i * SLOT, i);
			irq.byte_size = i * irq.slot_size;
			irq.unibuf = vmalloc(irq.byte_size);
			if (irq.unibuf) break;
		}
		irq.buffer_size = i;
	}

        if (!irq.unibuf) {
	        DOUBLE_LOG("%s:%s failed to allocate buffer\n", HERE);
	        ret_val = -ENOMEM;
	        goto write_return;
        }

	/* set the timer period using the board call-back */

	(* daq_ops.set_timer) (irq.cadence_usec);

        /*  if only preset has been requested, return now  */

        if (command.code[0] == 'p') goto write_return;

	spm_device_action = Ready;

	dprintk (KERN_ALERT "%s:%s - scan device is armed"
			" and ready to start\n", HERE)

 write_return:
	up (&spm_device_in_use);
	return ret_val;
}
