/*
 * interrupt handler
 */

unsigned int up_byte (unsigned char b) {
  return (((b & 0x38) << 5) + ((~b & 0x80) << 4));
}

int irq_handler (int irq,
		 void * dev,
		 struct pt_regs *regs) {

  char * nam = "latim_pp:irq_handler";
  unsigned int te;
  struct timespec now;
  unsigned long spin_temp;
  
  /* process the interrupt and store data */

  switch (irq_action) {

  case (Go_Process):

    outb (pp_count_freeze | PIRQ_ENABLE, base+2);  /* freeze the counter */
    getnstimeofday (&now);                        /* read the clock */
    te = inb(base) + up_byte(inb(base+1));         /* read the counter */
    outb (pp_count_run | PIRQ_ENABLE, base+2);     /* unlock the counter */

    spin_lock_irqsave (&irq_lock, spin_temp);
  
    if (irq_buffer_count == irq_buffer_size) {
      printk (KERN_ALERT
	      "%s - irq buffer overrun with %d events pending\n",
	      nam, irq_awake_in);
      irq_action = Idle;
      outb(PIRQ_DISABLE, base + 2);             /* disable interrupts */
    } else {

      (irq_unibuf + irq_write_pointer)->timic = now;
      (irq_unibuf + irq_write_pointer)->timec = te * pp_counter;

      ++irq_buffer_count;
      if (++irq_write_pointer == irq_buffer_size) irq_write_pointer = 0;
   
      if (irq_awake_in && --irq_awake_in <= 0) {
	ready_to_read = 1;
	wake_up_interruptible(&latim_pp_queue);
      }
    }

    spin_unlock_irqrestore (&irq_lock, spin_temp);
    return IRQ_HANDLED;

  case (Idle):
    outb(PIRQ_DISABLE, base + 2);               /* disable interrupts */
    printk (KERN_ALERT "%s - unsolicited interrupt on line %d\n", NAME, irq);
    return IRQ_HANDLED;
  }

  printk (KERN_ALERT "%s - AAARGHHH??!!! - UNKNOWN INTERRUPT REQUEST\n", nam);

  return IRQ_HANDLED;
}

