/*
 * register_irq_handler
 *
 *   shared == no  :  register exclusive irq line or fail
 *   shared == yes :  register shared irq line or fail
 *   shared == try :  try first shared = "no", if failed try shared = "yes"
 *
 *   default is "try"
 *
 *   return: irq line or error code
 */

static int irq=0;
module_param (irq, int, S_IRUGO);

static char * shared="try";
module_param (shared, charp, S_IRUGO);

static int register_irq_handler (void * device, struct pci_dev *pci_device,
   		                  void * nibac_irq_handler) {

	int result=1;
	int irq_line;
	
	if (irq == 0) {
		irq_line = pci_device->irq;
	} else {
		/* statically assigned irq line */
		irq_line = irq;
	}

	if (irq_line == 0) {
		printk (KERN_ALERT "%s:%s - No valid irq line for "
			"device 0x%x\n", HERE, pci_device->device);
		return -EINVAL;
	}

	printk (KERN_ALERT "%s:%s - device %4.4x has irq %d\n",
		HERE, pci_device->device, irq_line);

	if (!strcmp(shared,"try") || !strcmp(shared,"no"))
		result = request_irq (irq_line,
				      (void *) nibac_irq_handler,
				      IRQF_DISABLED,
				      NAME, (void *) device);

	if (result == 0) {
		shared = "no";
	} else if (!strcmp(shared, "try") || !strcmp(shared,"yes")) {
		result = request_irq (irq_line,
				      (void *) nibac_irq_handler,
				      IRQF_DISABLED | IRQF_SHARED,
				      NAME, (void *) device);
		if (result == 0) shared = "yes";
	}

	if (result) {
		printk(KERN_ALERT "%s:%s - can't register irq %d for "
		       "device 0x%x: %d\n", HERE, irq_line,
		       pci_device->device, result);
		irq_line = 0;
		return -EINVAL;
	}

	irq = irq_line;
	printk (KERN_ALERT "%s:%s - Registered irq %d for device 0x%x "
		"with id %p\n", HERE, irq_line,
		pci_device->device, device);
	
	return irq_line;
}

