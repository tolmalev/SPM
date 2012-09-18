/*
 *     int nibac_open (struct inode *inode, struct file *filp)
 *
 */

int nibac_open (struct inode *inode, struct file *filp) {

	printk (KERN_ALERT "%s:%s - pid %d  inode %p  file %p  num %d:%d "
		"- open\n", HERE, current->pid, inode, filp,
		MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

        /* save the nibac_pci_device structure address of the device
           we are working with, for later use by other module functions */

	filp->private_data = (void *)
		container_of (inode->i_cdev, struct nibac_pci_device, cdev);
	  
	return 0;
}

/*
 *  void nibac_release (struct inode *inode, struct file *filp)
 *
 */

int nibac_release (struct inode *inode, struct file *filep) {

	printk (KERN_ALERT "%s:%s - pid %d  inode %p  file %p  num %d:%d "
		"- close\n", HERE, current->pid, inode, filep,
		MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	return 0;
}
