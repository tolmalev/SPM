/*
 *     test the external counter (if any)
 */

int flips(int bit_code) {
        int ntry=20, counter[ntry], j, n=0, m=0;
        outb(bit_code | PIRQ_DISABLE, base+2);
        for ( j=0 ; j<ntry ; j++ )
                counter[j] = inb(base) + up_byte(inb(base+1));
        for ( j=1 ; j<ntry ; j++ ) {
                if (counter[j] != counter[j-1]) n++;
                if ((counter[j] & 0xff) != (counter[j-1] & 0xff)) m++;
        }
        return (m ? n : -n);
}

/*
 * int latim_pp_open (struct inode *inode, struct file *filp)
 *
 */

int latim_pp_open (struct inode *inode, struct file *filep) {

        unsigned long spin_temp;
        int try_run, try_freeze;

        do_gettimeofday(&latim_pp_o);

        printk (KERN_WARNING "%s:%s - open request from %i\n",
                HERE, current->pid);

        /*
         *    mutex checkpoint to block a second open request
         */

        if (down_interruptible (&latim_pp_device_available) != 0) {
                printk (KERN_WARNING "%s:%s - request for %i interrupted\n",
                        HERE, current->pid);
                return -EINTR;
        }

        printk (KERN_WARNING "%s:%s - device opened for %i\n",
                HERE, current->pid);

        /*
         *    allocate circular buffer memory
         */

        if (!(irq_unibuf = vmalloc(buffer_size * SLOT))) {
                DOUBLE_LOG ("%s:%s - failed to allocate buffer\n", HERE);
                up (&latim_pp_device_available);
                return -ENOMEM;
        } 

        /*
         *    check for an external counter on data lines of PP
         *    and prepare freeze codes
         */

        pp_count_run = 1 << (read_bit - 1);
        pp_count_freeze = 0;
        try_run = flips(pp_count_run);
        try_freeze = flips(0);

        if (try_freeze == 0 && try_run > 0) {
                DOUBLE_LOG ("%s:%s - counter found with run mask: %x\n",
                            HERE, pp_count_run);
        } else if (try_freeze > 0 && try_run == 0) {
                pp_count_freeze = pp_count_run;
                pp_count_run = 0;
                DOUBLE_LOG ("%s:%s - counter found with freeze mask: %x\n",
                            HERE, pp_count_freeze);
        } else if (try_freeze == 0 && try_run == 0) {
                DOUBLE_LOG ("%s:%s - no external counter detected\n", HERE);
                //    pp_count_run = 0;
        } else if ((try_freeze < 0) ^ (try_run < 0)) {
                DOUBLE_LOG ("%s:%s - %s\n", HERE,
                            "External counter found, but PP is write only");
        } else {
                DOUBLE_LOG ("%s:%s - could not freeze the external counter\n",
                            HERE);
                //    pp_count_run = 0;
        }
        pp_counter = pp_count_run || pp_count_freeze;
    
        spin_lock_irqsave (&irq_lock, spin_temp);

        irq_write_pointer = 0;
        irq_read_pointer = 0;
        irq_awake_in = 0;
        irq_buffer_count = 0;
        irq_buffer_size = buffer_size;
        ready_to_read = 0;
        irq_action = Go_Process;
        outb(pp_count_run | PIRQ_ENABLE, base+2);   /* interrupt line enable */

        spin_unlock_irqrestore (&irq_lock, spin_temp);

        return 0;
}

/*
 *  void latim_pp_release (struct inode *inode, struct file *filp)
 *
 */

int latim_pp_release (struct inode *inode, struct file *filep) {

        int action;
        unsigned long spin_temp;

        spin_lock_irqsave (&irq_lock, spin_temp);
        {
                /* disable interrupts and clear flags */
                outb(PIRQ_DISABLE | pp_count_run, base + 2);
                action = irq_action;
                irq_action = Idle;
        }
        spin_unlock_irqrestore (&irq_lock, spin_temp);

        if (action != Idle) {
                printk (KERN_ALERT "%s:%s - killed the loop while active\n",
                        HERE);
        }

        vfree(irq_unibuf);
        up (&latim_pp_device_available);

        printk (KERN_WARNING "%s:%s - device closed for %i\n",
                HERE, current->pid);
        return 0;
}
