/*
 *    ltm_ppi - latency time measurements by parallel port interrupt
 */
    
/*
 * Copyright (C) 2007 Marcello Carla'
 *
 * This program is free software; you can use, modify and redistribute
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (www.fsf.org).
 *
 * Address of the author:
 * Department of Physics, University of Florence
 * Via G. Sansone 1
 * Sesto F.no (Firenze) I 50019 - Italy
 * e-mail: carla@fi.infn.it
 *
 */

/* base module includes */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>


/* character device */

#include <linux/cdev.h>

/* interrupts and time*/

#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/spinlock.h>

/* access to parallel port registers */

#include <asm/io.h>
#include <linux/device.h>
#include <linux/ioport.h>

#include "latim_pp.h"
#include "../include/latim_pp_data.h"

MODULE_LICENSE("GPL v2");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#error This version of nirlpk has only been tested on 2.6 kernels.
#endif

/*
 *    parameters
 */

#define NAME "latim_pp"
#define PIRQ_ENABLE 0x31
#define PIRQ_DISABLE 0x21
#define PORTS_NUMBER 3

static int irq=7;
module_param (irq, int, S_IRUGO);

static int base=0x378;
module_param (base, int, S_IRUGO);

static int major = 0, minor = 0;
module_param (major, int, S_IRUGO);
module_param (minor, int, S_IRUGO);

static int timeout = 2000;   /* msec */
module_param (timeout, int, S_IRUGO | S_IWUSR);

static int buffer_size = 100000;
module_param (buffer_size, int, S_IRUGO | S_IWUSR);

static int read_bit = 2;
module_param (read_bit, int, S_IRUGO | S_IWUSR);

static int high_water = 70;
module_param (high_water, int, S_IRUGO | S_IWUSR);

/* two mutex are used:
 * 'nibac_device_available' blocks a second process trying to open the device; 
 * 'nibac_device_in_use' blocks a second concurrent operation;
 * wait queue 'nibac_queue' is used to wait for the end of the operation
 * carried on in the interrupt system.
 * Spinlock protects the data shared between read/write function and
 * interrupt handler: irq_action
 */

DECLARE_MUTEX(latim_pp_device_available);
DECLARE_MUTEX(NAME_device_in_use);
DECLARE_WAIT_QUEUE_HEAD(latim_pp_queue);

/*
 * shared resources for irq service routine
 */

static spinlock_t irq_lock = SPIN_LOCK_UNLOCKED;

DECLARE_MUTEX (any_irq_read_device);

static enum {Idle, Go_Process} irq_action = Idle;

static struct event * irq_unibuf = NULL;

static int irq_buffer_size;    /* total buffer size (in slots) */
static int irq_write_pointer;  /* in slots */
static int irq_read_pointer;   /* in bytes */
static int irq_awake_in;
static int irq_buffer_count;   /* slots (or part of a slot) in buffer */ 
static int ready_to_read=0;

static int pp_count_freeze;
static int pp_count_run;
static int pp_counter;

/* 
 *  Globals 
 */

int latim_pp_open    (struct inode *, struct file *);
int latim_pp_release (struct inode *, struct file *);
ssize_t latim_pp_read   (struct file *, char __user *, size_t, loff_t *);
ssize_t latim_pp_write  (struct file *, const char __user *, size_t, loff_t *);

static struct file_operations latim_pp_fops = 
{
	.open    = latim_pp_open,
	.release = latim_pp_release,
	.read    = latim_pp_read,
	/*    .write   = latim_pp_write,   */
};

#define MINOR_COUNT 1

static dev_t device_number;
static struct cdev cdev;
static dev_t devnumber;

static struct timeval latim_pp_o, latim_pp_r;

#include "latim_pp_interrupt_handler.c"
#include "latim_pp_init_exit.c"
#include "latim_pp_open_release.c"
#include "latim_pp_read_write.c"
