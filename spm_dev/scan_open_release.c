/*
 * int scan_open (struct inode *inode, struct file *filp)
 *
 * The 'open' function is used to block a second access request
 * to the scan device. The process issuing the request is blocked
 * until the device is released, or the request is interrupted.
 * If the open request is O_NONBLOCK, 'open' returns immediately
 * with -EBUSY.
 *
 */

int scan_open (struct inode *inode, struct file *filep) {

	dprintk (KERN_WARNING "%s:%s - open request from %i\n",
		 HERE, current->pid);

	/* mutex checkpoint to block a second open request */

	GET_THE_LOCK (filep, spm_device_available);

	dprintk (KERN_WARNING
		 "%s:%s - device opened for %i\n", HERE, current->pid);
  
	return 0;
}

/*
 *  void scan_release (struct inode *inode, struct file *filp)
 *
 * The feedback loop operation is armed by a 'write' to /dev/spm/scan and
 * is actually started by the first 'read' operation from 'dev'spm/scan.
 * This allows a task to issue the 'write' (echo 's' > /dev/spm/scan) and
 * another task to read the data (od -w52 -t x4 /dev/spm/scan).
 * When the device is released after having read something, every pending
 * operation is canceled.
 *
 */

int scan_release (struct inode *inode, struct file *filep) {

	int action;
	unsigned long spin_temp;

	dprintk (KERN_ALERT
		 "%s:%s - closing device for %i\n", HERE, current->pid);

	spin_lock_irqsave (&irq_lock, spin_temp);
	{
		action = irq.action;
		irq.action = Idle;
	}
	spin_unlock_irqrestore (&irq_lock, spin_temp);

	if (action != Idle) {
                DOUBLE_LOG ("%s:%s - killed the loop while active\n", HERE);
        }

	if (spm_device_action == Working) spm_device_action = Nop;  

	up (&spm_device_available);

	dprintk (KERN_ALERT
		 "%s:%s - device closed for %i\n", HERE, current->pid);
	return 0;
}
