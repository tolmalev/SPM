/*
 * latim_pp_init - module initialization
 *
 */

static int latim_pp_init (void) {

        int status = 0;

        printk (KERN_ALERT "%s - %s %s - init, PAGE_SIZE %ld\n",
                NAME, __DATE__, __TIME__, PAGE_SIZE);

        /* get a device number range*/

        if (major)                /* static major number */ {
                device_number = MKDEV(major, 0);
                status = register_chrdev_region
                        (device_number, MINOR_COUNT, NAME); 
        } else                    /* dynamic major number */ {
                status = alloc_chrdev_region
                        (&device_number, 0, MINOR_COUNT, NAME);
                major = MAJOR(device_number);
        }

        if (status < 0) {
                printk (KERN_ALERT 
                        "%s - Could not register numbers for 'latim_pp'\n",
                        NAME);
                return status;
        }

        printk (KERN_ALERT "%s - latim_pp numbers: major %i  minor %d\n",
                NAME, major, MINOR_COUNT);

        /* get exclusive use of registers area */

        if (!request_region(base, PORTS_NUMBER, NAME)) {
                printk (KERN_ALERT "%s: Cannot get I/O registers at 0x%x\n",
                        NAME, base);
                unregister_chrdev_region (device_number, MINOR_COUNT);
                return -EBUSY;
        }

        if (irq > 0) {

                outb(PIRQ_DISABLE, base + 2); /* make sure irq is disabled */
                status = request_irq(irq, (void *) irq_handler,
                                     IRQF_DISABLED, NAME, NULL);
                if (status) {
                        printk(KERN_ALERT "%s: can't get irq %i\n", NAME, irq);
                        release_region (base, PORTS_NUMBER);
                        unregister_chrdev_region (device_number, MINOR_COUNT);
                        return status;
                }
                printk (KERN_WARNING "%s: allocated interrupt %i\n",
                        NAME, irq);
        }


        /*
         *  create and register the device
         */

        devnumber = MKDEV(major, 0);
        cdev_init (&cdev, &latim_pp_fops);
        cdev.owner = THIS_MODULE;
	  
        if (kobject_set_name (&cdev.kobj, "latim_pp") >=0 ) {
                if (cdev_add (&cdev, devnumber, 1) >= 0) {
                        printk (KERN_ALERT "%s - latim_pp driver installed\n",
                                NAME);
                        return 0;
                }
        }

        unregister_chrdev_region ( device_number, MINOR_COUNT);
        printk (KERN_ALERT "%s - driver initialization failed: %i\n",
                NAME, status);
        return -EINVAL; 
}

/*
 * latim_pp_exit - module removal
 *
 */

static void latim_pp_exit (void) {
 
        printk (KERN_ALERT "%s - unloading module\n", NAME);

        if (irq >= 0) {
                outb(PIRQ_DISABLE, base + 2);
                free_irq(irq, NULL);
        }

        cdev_del(&cdev);
        release_region (base, PORTS_NUMBER);
        unregister_chrdev_region (device_number, MINOR_COUNT);
    
        printk (KERN_ALERT "%s - module unloaded\n", NAME); 
}

module_init (latim_pp_init);
module_exit (latim_pp_exit);

