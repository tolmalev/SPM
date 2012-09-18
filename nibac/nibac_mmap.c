/*
 * memory map management
 */
 
static void shadow_vma_open (struct vm_area_struct *);
static void shadow_vma_close (struct vm_area_struct *);
static int  shadow_vma_fault (struct vm_area_struct *,
				       struct vm_fault *);

static struct vm_operations_struct shadow_vmops = {
	.open   = shadow_vma_open,
	.close  = shadow_vma_close,
	.fault  = shadow_vma_fault
};

/*
 *
 * int nibac_mmap(struct file *filep, struct vm_area_struct *vma)
 *
 *
 */ 

int nibac_mmap(struct file *filep, struct vm_area_struct *vma) {

	unsigned long physicaladdr = 0;
	unsigned long vsize = vma->vm_end - vma->vm_start;
	struct nibac_pci_device * device = filep->private_data;

	int ibar;

	printk (KERN_ALERT
		"%s:%s vm_start: %lx  vm_end: %lx  vm_pgoff:%lx  file: %p\n",
		HERE, vma->vm_start, vma->vm_end, vma->vm_pgoff, filep);

	/*
	 * vm_pgoff conventionally describes the area to be mapped:
	 *
	 * 0 ... MMAP_REGIONS : registers area
	 * MMAP_REGIONS ... 2 * MMAP_REGIONS : shadow area
	 *
	 */

	if ((ibar=vma->vm_pgoff) < MMAP_REGIONS) {
		if (ibar >= NUMBER_OF_BARS) goto bad_map;
		  
		physicaladdr = pci_resource_start
			(device->pci_device, device->bar[ibar]);
		vma->vm_pgoff = physicaladdr >> PAGE_SHIFT;

		printk (KERN_ALERT
			"%s:%s - m'mapping bar %d at physical address %lx\n",
			HERE, ibar, physicaladdr);

		if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				   vsize, vma->vm_page_prot) == 0) return 0;
 
		printk (KERN_ALERT "%s:%s - m'mapping failed\n", HERE);
		return -EFAULT;

	} else if (ibar < 2 * MMAP_REGIONS) {

		ibar -= MMAP_REGIONS;
		if (ibar >= NUMBER_OF_BARS) goto bad_map;

		printk (KERN_ALERT "%s:%s - m'mapping shadow for bar %d\n",
			HERE, ibar);

		vma->vm_ops = & shadow_vmops;

		vma->vm_private_data = device->memap[ibar].shadow;

		printk (KERN_ALERT "%s:%s - shadow address is %p\n",
			HERE, vma->vm_private_data);

	} else if (ibar == 2 * MMAP_REGIONS + 1) {

		printk (KERN_ALERT "%s:%s - m'mapping calibration area %d\n",
			HERE, ibar);

		vma->vm_ops = & shadow_vmops;

		vma->vm_private_data = device->calibration;

		printk (KERN_ALERT "%s:%s - calibration area address is %p\n",
			HERE, vma->vm_private_data);

	}
	return 0;

	/*
	 *    the offset refers to a DMA buffer
	 */

	printk (KERN_ALERT "%s:%s - DMA buffer not supported\n", HERE);
	return -ENXIO;

 bad_map:
	printk (KERN_ALERT "%s:%s - m'mapping non-existent area %ld\n",
		HERE, vma->vm_pgoff);
	return -ENXIO;
}


/*
 * functions for register shadow m'mapping
 */

static void shadow_vma_open (struct vm_area_struct * vma) {
	printk (KERN_ALERT
		"%s:%s - shadow mapping open - virtual %lx, physical %lx\n",
		HERE, vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

static void shadow_vma_close (struct vm_area_struct *vma) {
	printk (KERN_ALERT
		"%s:%s - shadow mapping close - virtual %lx, physical %lx\n",
		HERE, vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

static int shadow_vma_fault (struct vm_area_struct * vma,
			     struct vm_fault *vmf) {

	unsigned long physaddr = (unsigned long) vmf->virtual_address -
                vma->vm_start + (unsigned long) vma->vm_private_data;

        vmf->page = virt_to_page (physaddr);

	get_page(vmf->page);

        printk (KERN_ALERT"%s:%s - mapped page %p at %lx with count %d\n",
		HERE, vmf->page, physaddr, page_count(vmf->page));

	return  VM_FAULT_MINOR;
}
