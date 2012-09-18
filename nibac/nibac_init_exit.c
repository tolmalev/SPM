/*
 * nibac_dev_init - module initialization
 *
 */

static int nibac_init (void) {

	int status = 0;

	printk (KERN_ALERT "\n%s:%s - %s %s - init, PAGE_SIZE %ld\n",
		HERE, __DATE__, __TIME__, PAGE_SIZE);

	/*
	 *    identify and register DAQ boards
	 */

	max_boards = 0;

	status = pci_register_driver(&nibac_pci_driver);

	if (status < 0) {
		pci_unregister_driver(&nibac_pci_driver);
		printk (KERN_ALERT "%s:%s  driver initialization failed: %i\n",
			HERE, status);
	}

	return status; 
  
}

/*
 * nibac_dev_exit - module removal
 *
 */

static void nibac_exit (void) {

	printk (KERN_ALERT "%s:%s - unloading module\n", HERE);

	pci_unregister_driver(&nibac_pci_driver);

	printk (KERN_ALERT "%s:%s - module unloaded\n", HERE); 
}

module_init (nibac_init);
module_exit (nibac_exit);
