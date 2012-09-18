/*
 *    nibac.c - driver for National Instruments DAQ boards
 *
 *    supported boards:
 *
 *      1093:70af : PCI-6221
 *      1093:71bc : PCI-6221 37 pin
 *      1093:28c0 : 
 */
    
/*
 * Copyright (C) 2008 Marcello Carla'
 *
 * This program is free software; you can use, modify and redistribute
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (www.fsf.org).
 *
 * Address of the author:
 * Department of Physics, University of Florence
 * Via G. Sansone 1
 * Sesto F.no (Firenze) I 50019 - Italy
 * e-mail:
 *
 */
#define DEBUG 0

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#error This code has only been tested on 2.6 kernels.
#endif

/* base module includes */
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

/* for PCI */
#include <linux/autoconf.h>  
#include <linux/pci.h>

/* character device */
#include <linux/cdev.h>

/* proc file support */
#include <linux/proc_fs.h>

/* copy_to_user / copy_from_user */
#include <asm/uaccess.h>

/* interrupts and time*/
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL v2");

/* local 'include's */

#include "../include/spm_bac.h"
#include "../include/spm_dev_data.h"
#include "nibac.h"

/*
 * parameters, access and resources
 */

#ifndef CONFIG_PCI
#error "This driver requires PCI support."
#endif

#define NAME "nibac"

/*
 *    board_rank[0] = number of board to be master
 *    board_rank[1] = number of board to be slave
 *
 */

static int board_rank[MAX_BOARDS]={1,2}, board_minor[MAX_BOARDS];
static int max_boards = MAX_BOARDS;
module_param_array(board_rank, int, &max_boards, S_IRUGO);
module_param_array(board_minor, int, &max_boards, S_IRUGO);

static int major=0;
module_param (major, int, S_IRUGO);

static LIST_HEAD(device_list);
static DECLARE_MUTEX(device_lock);

/* 
 *   Data area for DAQ boards
 */

static const struct pci_device_id nibac_known_pci_device[ ] = {
	{PCI_DEVICE(0x1093,0x70af)},
	{PCI_DEVICE(0x1093,0x71bc)},
	{PCI_DEVICE(0x1093,0x28c0)},
	{ 0, },
};

MODULE_DEVICE_TABLE(pci, nibac_known_pci_device);

static struct pci_driver nibac_pci_driver = 
{
	.name        = NAME,
	.id_table    = nibac_known_pci_device,
	.probe       = nibac_probe_device,
	.remove      = nibac_remove_device,
	.suspend     = NULL,
	.resume      = NULL,
};

static struct file_operations nibac_fops = 
{
	.open    = nibac_open,
	.release = nibac_release,
	.mmap    = nibac_mmap,
	.write   = nibac_write,
	.read    = nibac_read,
};

/*
 * board register access
 */

u8  ** mwm=NULL, ** mwr=NULL, ** swm=NULL, ** swr=NULL;

void nibac_set_timer (int usec) {
	int irq_cadence = usec*20 -1;
	NIREG (mwr,AI_SI_Load_B,write,irq_cadence);
	NIREG (mwr,AI_SI_Load_A,write,irq_cadence);
}

void nibac_set_samples (int n) {

        printk (KERN_ALERT "%s:%s - %s\n", HERE,
                "Sorry, samples_per_point cannot be modified on-the-fly.");
        printk (KERN_ALERT "%s:%s - %s\n", HERE,
                "Internal data structures may have been corrupted.");
}

void nibac_set_adc (int n) {

        printk (KERN_ALERT "%s:%s - %s\n", HERE,
                "Sorry, sample_adc cannot be modified on-the-fly.");
        printk (KERN_ALERT "%s:%s - %s\n", HERE,
                "Internal data structures may have been corrupted.");
}

void nibac_second_write (struct event_head * data, int n) {

	if (data->n_dac > n)
		NIREG(swr, DAC_Direct_Data, 0, write, IRQ_DAC[n]);
	if (data->n_dac > n+1)
		NIREG(swr, DAC_Direct_Data, 1, write, IRQ_DAC[n+1]);
        NIREG(swr, Static_Digital_Output, write, data->byte[1]);
}

/* dma buffer */

static s16 * dma_buffer;
static ulong dma_address=0;
module_param (dma_address, ulong, S_IRUGO);
static int dma_use=0;
module_param (dma_use, int, S_IRUGO | S_IWUSR);

/* code sections */

#include "nibac_init_exit.c"
#include "register_irq_handler.c"
#include "nibac_probe_remove.c"

#include "nibac_open_release.c"
#include "nibac_mmap.c"
#include "nibac_interrupt_handler.c"

#include "nibac_read_write.c"
