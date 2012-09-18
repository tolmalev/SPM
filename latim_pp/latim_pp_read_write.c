/*
 *
 *    latim_pp_read - read a data buffer
 *
 */

ssize_t latim_pp_read  (struct file * filp,
                     char __user * buf,
                     size_t byte_count,
                     loff_t * offset) {

  u64 jft=0, jfti;
  int result;
  int not_copied;
  int ret_val;
  char * nam = "latim_pp:secm_read";
  int up_slot, down_slot;
  unsigned long spin_temp;
  int start_slot, free_slots;
  int buffer_count;
  int tmo = timeout * HZ / 1000;
  int byte_stored, byte_needed;
  int byte_in_buffer, byte_transfer;
  int  go_to_sleep, old_buffer_count;
  int high_water_count;

  do_gettimeofday(&latim_pp_r);

  /*
  printk (KERN_ALERT "%s - read request for device: %d:%d after %d\n", nam,
          MAJOR(filp->f_dentry->d_inode->i_rdev),
          MINOR(filp->f_dentry->d_inode->i_rdev),
	  usec_diff(&latim_pp_r, &latim_pp_o));
  */

  if (down_interruptible (&any_irq_read_device) != 0) {
    printk (KERN_WARNING "%s: request for %i interrupted\n",
            nam, current->pid);
    return -EINTR;
  }

  /*
  printk(KERN_ALERT "%s: requested %d bytes\n", nam, (int)byte_count);
  */

  byte_stored = 0;

  while (1) {

    /* if some data in irq_buffer, move it to user space */

    spin_lock_irqsave (&irq_lock, spin_temp);
    buffer_count = irq_buffer_count;
    spin_unlock_irqrestore (&irq_lock, spin_temp);

    byte_needed = byte_count - byte_stored;
    byte_in_buffer = buffer_count * SLOT - irq_read_pointer % SLOT;
    if (byte_in_buffer < 0) byte_in_buffer += irq_buffer_size * SLOT;

    if (byte_in_buffer) {
      byte_transfer =
	byte_in_buffer > byte_needed ? byte_needed : byte_in_buffer;

      /* how many bytes up and down the circular buffer */

      up_slot = irq_buffer_size * SLOT - irq_read_pointer;
      if (up_slot > byte_transfer) {
	up_slot = byte_transfer;
	down_slot = 0;
      } else {
	down_slot = byte_transfer - up_slot;
      }

      /* copy to user buffer */

      not_copied = copy_to_user(buf + byte_stored, 
				(u8 *)irq_unibuf + irq_read_pointer, up_slot);
      if (!not_copied && down_slot)
        not_copied += copy_to_user(buf + byte_stored + up_slot,
				   irq_unibuf, down_slot);

      if (not_copied) {
        printk (KERN_ALERT "%s: Error while copying to user space\n", nam);
        ret_val = -EFAULT;
        break;
      }

      /* now let's update all pointers */

      byte_stored += byte_transfer;

      start_slot = irq_read_pointer / SLOT;
      irq_read_pointer += byte_transfer;
      free_slots = irq_read_pointer / SLOT - start_slot;

      if (irq_read_pointer >= irq_buffer_size * SLOT)
        irq_read_pointer -= irq_buffer_size * SLOT;

      /* atomically update irq_buffer_count */

      spin_lock_irqsave (&irq_lock, spin_temp);
      irq_buffer_count -= free_slots;
      spin_unlock_irqrestore (&irq_lock, spin_temp);

      if (byte_count == byte_stored) {  /* return with a valid full buffer */
	ret_val = byte_count;
	break;
      }
    }

    /* more data are needed */

    high_water_count = (high_water * buffer_size) / 100;
    spin_lock_irqsave (&irq_lock, spin_temp);
    {
      ready_to_read = 0;
      byte_in_buffer = irq_buffer_count * SLOT - irq_read_pointer % SLOT;
      if (byte_in_buffer < 0) byte_in_buffer += irq_buffer_size * SLOT;
      byte_needed = byte_count - byte_stored - byte_in_buffer;
      if (byte_needed > 0) {
	irq_awake_in = (byte_needed + PAD) / SLOT;
	if (irq_awake_in + irq_buffer_count > high_water_count)
	  irq_awake_in = high_water_count - irq_buffer_count;
      } else {
	irq_awake_in = 0;
      }
      go_to_sleep = irq_awake_in;
      old_buffer_count = irq_buffer_count;
    }
    spin_unlock_irqrestore (&irq_lock, spin_temp);

    /*
    printk (KERN_ALERT "%s - going to sleep for %d events\n",
	    nam, irq_awake_in);
    */

    if (go_to_sleep) {
      jfti = get_jiffies_64();
      result = wait_event_interruptible_timeout (latim_pp_queue,
						 ready_to_read!=0, tmo);
      jft = get_jiffies_64() - jfti;
    } else {
      continue;           /* data already in buffer; process them */
    }

    /* check if we received a ctrl^C or a similar signal */

    if (ready_to_read == 0 && result != 0) {
      printk (KERN_ALERT "%s: interrupt signal\n", nam);
      ret_val = -EINTR;
      break;
    }

    if (ready_to_read == 0 && result == 0) {         /* timeout */
      spin_lock_irqsave (&irq_lock, spin_temp);
      buffer_count = irq_buffer_count;
      spin_unlock_irqrestore (&irq_lock, spin_temp);
      if (buffer_count == old_buffer_count) {      /* true timeout */
	ret_val = -ETIME;
	break;
      }
      continue;   /* thwere is some progress - give another chance */
    }
  }

  if (ret_val < 0) {                /* data transfer failed or eof */
    spin_lock_irqsave (&irq_lock, spin_temp);
    irq_action = Idle;
    spin_unlock_irqrestore (&irq_lock, spin_temp);
  }

  if (ret_val > 0) do_gettimeofday(&latim_pp_o);   /* data transfer valid */

  up (&any_irq_read_device);
  return ret_val;
}
