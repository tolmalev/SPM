/*
 *      nibac_probe_device - initialize PCI device
 *
 *      Called on module initialization, for every device declared
 *      in nibac_known_pci_device and found.
 *      We need resources in this order:
 *
 *      - memory for device structure
 *      - minor number from reserved number pool
 *      - memory regions for BARs
 *      - mapping of memory regions
 *      - memory for shadow registers
 *      - interrupt handler registration
 *      - pci device enable
 *      - device registration
 *      - device insertion into local list
 *      
 */
 
static int nibac_probe_device (struct pci_dev *pci_device,
			       const struct pci_device_id *id) {

	int dev_count=0;
	int status, i;
	struct nibac_pci_device *device = NULL;
	struct list_head *pos;
	static int board_count=0;        /* counter of the proposed devices */

	/* check for an impossible but disastrous condition */

	if (!pci_device) {
		printk (KERN_ALERT "%s:%s - AAARGH!!! called with NULL "
			"argument\n", HERE);
		return -EINVAL;
	}

	/* claim this board? */

	board_count++;
	for ( i=0 ; i<MAX_BOARDS ; i++ ){
		if (board_rank[i] == board_count) goto claim_this_board;
	}
	return -EINVAL;                   /* we do not use this board */

 claim_this_board:

	__list_for_each (pos, &device_list) dev_count++;

	/* allocate memory for device data structure */

	device = (struct nibac_pci_device *)
		kzalloc (sizeof (struct nibac_pci_device), GFP_KERNEL);

	if (!device) {
		printk (KERN_ALERT "%s:%s - Could not kmalloc space for "
			"device!\n", HERE);
		return -ENOMEM;
	}

	device->rank = i;

	/* initialize the data structure for this device */

	if ((device->devnumber = spm_global.get_devnumber()) == -1) {
		printk (KERN_ALERT
			"%s:%s - Could not get a minor number for device "
			"%d: %04x:%04x\n", HERE, dev_count, id->vendor,
			id->device);
		status = -ENODEV;
		goto bad_probe;
	}

	board_minor[device->rank] = MINOR(device->devnumber);
	major = MAJOR(device->devnumber);

	if (device->rank+1 > max_boards) max_boards = device->rank + 1;

	printk (KERN_ALERT "%s:%s - probing device #%d: 0x%04x:0x%04x - "
		"%d %d\n",
		HERE, dev_count, id->vendor, id->device,
		major, board_minor[device->rank]);

	INIT_LIST_HEAD(&device->link);
	device->pci_device = pci_device;

	if (device->rank == 0) {
		printk (KERN_ALERT "%s:%s - master board, %04x:%04x\n",
			HERE, id->vendor, id->device);
	} else {
		printk (KERN_ALERT "%s:%s - slave board, %04x:%04x\n",
			HERE, id->vendor, id->device);

	}

	/* first of all enable the device,
	   then set up regions for register access */

	if (pci_enable_device(pci_device) < 0) {
		status = -EINVAL;
		goto bad_probe;
	}

	device->bar[0] = 0;
	device->bar[1] = 1;

	for ( i=0 ; i < NUMBER_OF_BARS ; i++ ) {

		snprintf (device->name, NAME_SIZE, "NI-0x%4x:%d",
			  pci_device->device, i);

		device->memory_region[i] =
			pci_request_region (pci_device, device->bar[i],
					    device->name);

		if (device->memory_region[i] == 0) {
			device->memory_region[i] = 1;
		} else {
			printk (KERN_ALERT "%s:%s - allocation of region %d "
				"for device 0x%x failed\n",
				HERE, device->bar[i], pci_device->device);
			status = -EINVAL;
			goto bad_probe;
		}

		if (!(device->memap[i].registers =
		      pci_iomap (pci_device, device->bar[i], 0))) {
			printk (KERN_ALERT "%s:%s - pci_iomap for bar %d "
				"failed\n",HERE, i);
			status = -EINVAL;
			goto bad_probe;
		}

		printk (KERN_ALERT "%s:%s - bar %d  region %d -> %s   "
			"iomap %p  %s\n",
			HERE, i, device->bar[i],
			device->memory_region[i] ? "ok" : "fail",
			device->memap[i].registers, device->name);

		/* allocate memory for shadow registers */

		device->memap[i].shadow = (u8 *) get_zeroed_page(GFP_KERNEL);

		if (!device->memap[i].shadow) {
			printk (KERN_ALERT "%s:%s - kmalloc for shadow of BAR "
				"%d failed\n", HERE, i);
			status = -ENOMEM;
			goto bad_probe;
		}
	}

	/* allocate memory for calibration area */

	device->calibration = (u8 *) get_zeroed_page(GFP_KERNEL);

        if (!device->calibration) {
                printk (KERN_ALERT "%s:%s - kmalloc for calibration area "
                        "failed\n", HERE);
                status = -ENOMEM;
                goto bad_probe;
        }
  
	snprintf (device->name, NAME_SIZE, "NI-0x%4x", pci_device->device);
  
	/* activate bar1 in MITE chip */

	iowrite32 ((pci_resource_start(pci_device, 1) & 0xffffff00L) | 0x80,
		   device->memap[0].registers + 0xc0);

	/* load pointers for the interrupt handler */
  
	if (device->rank == 0) {

		mwm = (u8 **) &device->memap[0];    /* mite registers */
		mwr = (u8 **) &device->memap[1];    /* function registers */

		printk (KERN_ALERT "%s:%s - mwm: %p %p %p   mwr: %p %p %p\n",
			HERE, mwm, mwm[0], mwm[1], mwr, mwr[0], mwr[1]);

		printk (KERN_ALERT
			"%s - Board is master - Interrupt_Mask_2 is 0x%x\n",
			NAME, ioread32(device->memap[0].registers + 0x10 ));
	} else {

		swm = (u8 **) &device->memap[0];    /* mite registers */
		swr = (u8 **) &device->memap[1];    /* function registers */

		printk (KERN_ALERT "%s - swm,swr: %p %p\n", NAME, swm, swr);
	}

	/* setup cdev */

	cdev_init (&device->cdev, &nibac_fops);
	if (kobject_set_name (&device->cdev.kobj, "nibac%d",
			      MINOR(device->devnumber)) < 0) {
		status = -EINVAL;
		goto bad_probe;
	}

	if (device->rank == 0) {

		/* register the board as master */
		
		device->daq_ops =
                  spm_global.register_daq_board
                  (1, nibac_set_timer, nibac_set_samples, nibac_set_adc);
		if (device->daq_ops == NULL) {
			printk (KERN_ALERT "%s:%s - board not accepted as "
				"master by spm\n", HERE);
			status = -EINVAL;
			goto bad_probe;
		}
		printk (KERN_ALERT "%s:%s - spm functions: %p\n", HERE,
			device->daq_ops);

		/* register the interrupt handler */
		
		status = register_irq_handler (device, pci_device,
						 nibac_irq_handler);
		if (status < 0) goto bad_probe;
		device->irq_line = status;

		/* allocate DMA area - we use a static 4096 bytes area
                   for a 2*2048 bytes ping-pong buffer */

                if (pci_set_dma_mask (pci_device, DMA_64BIT_MASK) &&
                	pci_set_dma_mask (pci_device, DMA_32BIT_MASK)) {
                        if (spm_global.irq->samples_per_point != 1) {
                                DOUBLE_LOG ("%s:%s - %s\n", HERE,
                                            "No DMA; samples > 1 not allowed");
                                status = -EINVAL;
                                goto bad_probe;
                        }
                        
			dma_use = 0;
			DOUBLE_LOG ("%s:%s - %s\n", HERE,
				    "No DMA - back to direct FIFO read");
                } else {
			dma_buffer = pci_alloc_consistent
				(pci_device, 4096, &device->dma_phys_addr);
			if (!dma_buffer) {
				status = -ENOMEM;
				device->dma_phys_addr = 0;
				goto bad_probe;
			}
			dma_address = (ulong) (device->dma_phys_addr);
		}
		
                DOUBLE_LOG("%s:%s - DMA buffer virt.: %p   phys.: %lx\n",
			HERE, dma_buffer, dma_address);

	} else {
	
		/* register the board as slave */

		device->daq_ops = spm_global.register_daq_board
                  (0, nibac_second_write, NULL, NULL);
		if (device->daq_ops == NULL) {
			printk (KERN_ALERT "%s:%s - board not accepted as "
				"slave by spm\n", HERE);
		}
	}

	/* setup the pci device */

	pci_set_master(pci_device);
	pci_set_drvdata(pci_device, device);

	printk (KERN_ALERT "%s - pci device 0x%x set up\n",
		NAME, pci_device->device);

	/* initialize cdev object */
  
	device->cdev.owner = THIS_MODULE;

	if (cdev_add (&device->cdev, device->devnumber, 1) < 0) {
		status = -EINVAL;
		goto bad_probe;
	}

	printk (KERN_ALERT "%s - cdev for pci device 0x%x initialized\n",
		NAME, pci_device->device);

	/* if everything o.k, add device to list and return success */

	down(&device_lock);
	list_add(&device->link, &device_list);
	up(&device_lock);

	printk (KERN_ALERT "%s - device 0x%x completed with success\n",
		NAME, pci_device->device);

	return 0;

	/*
	 *     handle error conditions 
	 */

 bad_probe:
	release_resources (device);
	return status;
}

/*
 *
 *  nibac_remove_device - remove a pci device
 *  and release all allocated resources
 *
 */

static void nibac_remove_device(struct pci_dev *pci_device) {

	struct nibac_pci_device *device = NULL;
	u8 * mite_bar;
	unsigned long spin_temp;
	int status;

      
	printk (KERN_ALERT "%s:%s - Removing device 0x%x\n",
		HERE, pci_device->device);
    
	device = (struct nibac_pci_device*) pci_get_drvdata(pci_device);

	/* disable the board interrupt */

	if (device->rank == 0) {
		mite_bar = ((struct nibac_pci_device *)
			    device)->memap[0].registers;

		printk (KERN_ALERT "%s - disabling interrupt in board at %p\n",
			NAME, mite_bar);

		spin_lock_irqsave (spm_global.irq_lock, spin_temp);
		iowrite32(0x40150000, mite_bar+0x10);
		iowrite32(4, mite_bar+0x500);
		status = ioread32(mite_bar+0x53c);
		spin_unlock_irqrestore (spm_global.irq_lock, spin_temp);
		if (!status & 0x2000000) DOUBLE_LOG
			("%s:%s - ERROR: DMA not stopped: %x\n", HERE, status);
	}

	/* remove this device from device list */

	down(&device_lock);
	list_del(&device->link);
	up(&device_lock);

	release_resources (device);
}

/*
 *
 * release all resources allocated during device initialization
 *
 */

static void release_resources (struct nibac_pci_device *device) {

	int i;

	if (device == NULL) {
		printk (KERN_ALERT "%s:%s - called with "
			"NULL pointer\n", HERE);
		return;
	}

	if (device->daq_ops) spm_global.unregister_daq_board
				     (device->rank == 0);

	if (device->irq_line) {
		free_irq (device->irq_line, device);
		device->irq_line = 0;
	}

	for ( i=0 ; i < NUMBER_OF_BARS ; i++ ) {
                if (device->memory_region[i]) {
                        if (device->memap[i].registers) {
                                iounmap ((void *) device->memap[i].registers);
                                device->memap[i].registers = NULL;
                        }
                        if (device->memap[i].shadow) {
                                free_page ((ulong) device->memap[i].shadow);
                                device->memap[i].shadow = NULL;
                        }
                        pci_release_region
                                (device->pci_device, device->bar[i]);
                        device->memory_region[i] = 0;
                }
	}

        if (device->calibration) iounmap ((void *) device->calibration);
        device->calibration = NULL;

	if (device->pci_device) {
		pci_disable_device(device->pci_device);
		pci_set_drvdata(device->pci_device, NULL);
		device->pci_device = NULL;
	}

        /* free DMA area */

        if (device->dma_phys_addr)
                pci_free_consistent (NULL, 4096, dma_buffer,
                                     device->dma_phys_addr);

	/* cdev_del before cdev_add should be harmless
	   if right things are NULL */

	cdev_del(&device->cdev);

	/* device numbers are released globally at module unload */

	printk (KERN_ALERT "%s:%s - released device %s at %p\n", HERE,
		device->name, device);

        spm_global.release_devnumber(device->devnumber);
	kfree(device);
	device = NULL;
}
