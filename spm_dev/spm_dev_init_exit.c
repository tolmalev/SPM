/*
 *      spm_dev_init - module initialization
 */

static int spm_dev_init (void) {

	int status = 0;

	printk (KERN_ALERT "\n%s:%s - starting\n", HERE);
	  
	/* get a device number range*/

	if (scan_major) {                    /* static major number */
		device_number = MKDEV(scan_major, 0);
		status = register_chrdev_region
			(device_number, MINOR_COUNT, NAME); 
	} else {                    /* dynamic major number */
		status = alloc_chrdev_region (&device_number,
					      0, MINOR_COUNT, NAME);
		scan_major = MAJOR(device_number);
	}

	if (status < 0) {
		DOUBLE_LOG (
                        "%s:%s - register/alloc_chrdev_region failed: %i\n",
			HERE, status);
		return status;
	}
	printk (KERN_ALERT "%s:%s - registered major %i for %d minors\n",
		HERE, scan_major, MINOR_COUNT);

        /*
         *    preset slot_size and related values in irq table
	 *    and allocate for one event
         */

        irq.samples_per_point = samples_per_point;
        irq.sample_adc = sample_adc;
        irq.sample_dac = sample_dac;
	irq.cadence_usec = cadence_usec;

        irq.slot_size = sizeof (struct event_head) +
		2 * (irq.sample_adc * irq.samples_per_point + irq.sample_dac);
        slot = irq.slot_size;

	irq.data = kmalloc(irq.slot_size, GFP_KERNEL);

	if (!irq.data) {
                printk (KERN_ALERT "%s:%s - %s\n", HERE,
                        "Memory allocation for event data failed.");
                status = -ENOMEM;
                goto bad_exit;
	}

        irq.dac = (void *) irq.data + sizeof (struct event_head);
        irq.adc = irq.dac + irq.sample_dac;

	printk (KERN_ALERT "%s:%s - event_size: %d\n", HERE, irq.slot_size);

        irq.allow_new_slot = 0;

	/*
	 *    create the functionality devices
	 */

	cdev_init (&scan.cdev, &scan_fops);
	scan.cdev.owner = THIS_MODULE;

	cdev_init (&info.cdev, &info_fops);
	info.cdev.owner = THIS_MODULE;

	if ((scan.devnumber = get_devnumber()) == -1 ||
            (info.devnumber = get_devnumber()) == -1) {
		status = -EINVAL;
		DOUBLE_LOG (
			"%s - Could not register 'scan' device\n", NAME);
		goto bad_exit;
	}

	scan_minor =  MINOR(scan.devnumber);
	// info_minor = MINOR(info.devnumber);

	status = kobject_set_name (&scan.cdev.kobj, "scan");
	if (status < 0 ) goto bad_exit;

	status = kobject_set_name (&info.cdev.kobj, "info");
	if (status < 0 ) goto bad_exit;

	status = cdev_add (&scan.cdev, scan.devnumber, 1);
	if (status < 0 ) goto bad_exit;

	status = cdev_add (&info.cdev, info.devnumber, 1);
	if (status < 0 ) {
		cdev_del(&scan.cdev);
		goto bad_exit;
	}

	printk (KERN_ALERT "%s:%s - scan driver installed\n", HERE);
	return 0;
	
bad_exit:
        if (irq.data) kfree (irq.data);
	unregister_chrdev_region ( device_number, MINOR_COUNT);
	DOUBLE_LOG ("%s:%s - driver initialization failed: %i\n",
                    HERE, status);
	return status; 
}

/*
 *      spm_dev_exit - module removal
 */

static void spm_dev_exit (void) {

	printk (KERN_ALERT "%s:%s - unloading module\n", HERE);

	cdev_del(&info.cdev);
	cdev_del(&scan.cdev);
	unregister_chrdev_region ( device_number, MINOR_COUNT);

	if (irq.unibuf) vfree(irq.unibuf);
	irq.unibuf = NULL;

	if (irq.data) kfree (irq.data);
	irq.data = NULL;
    
	printk (KERN_ALERT "%s:%s - module unloaded\n", HERE); 
}

module_init (spm_dev_init);
module_exit (spm_dev_exit);
