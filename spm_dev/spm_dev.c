/*
 *    spm_dev - An Open Source package for digital feedback in 
 *              Scanning Probe Microscopy
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

/*
 * Circular buffer (irq.unibuf) allocation policy:
 * 1: try reuse the buffer in following scans
 * 0: give away the buffer as soon as possible
 */
#define HOLD_BUFFER 1

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#error This code has only been tested on 2.6 kernels.
#endif

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
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/autoconf.h>

/* character device */
#include <linux/cdev.h>

/* proc file support */
#include <linux/proc_fs.h>

/* copy_to_user / copy_from_user */
#include <asm/uaccess.h>

/* interrupts and time*/
#include <linux/interrupt.h>
//#include <linux/time.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL v2");

/* local 'include's */

#include "../include/spm_bac.h"
#include "../include/spm_dev_data.h"

/*
 * parameters, access and resources
 */

#define NAME "spm_dev"

#define MINOR_COUNT 8
#define MINIMAL_BUF 100

#define SLOT irq.slot_size
#define PAD (SLOT-1)
#define DEFAULT_CADENCE_USEC 200

static int scan_major=0;
module_param (scan_major, int, S_IRUGO);

static int scan_minor=0;
module_param (scan_minor, int, S_IRUGO);

static int timeout = 2000;   /* msec */
module_param (timeout, int, S_IRUGO | S_IWUSR);

static int buffer_size = 100000;
module_param (buffer_size, int, S_IRUGO | S_IWUSR);
static int buffer_size_min = 10000;
module_param (buffer_size_min, int, S_IRUGO | S_IWUSR);

static int high_water = 70;
module_param (high_water, int, S_IRUGO | S_IWUSR);

static int irq_stop_request = 0;
module_param (irq_stop_request, int, S_IRUGO | S_IWUSR);

static int lines_per_frame=512;
static int points_per_line=512;
static int samples_per_point=1;
static int cadence_usec=DEFAULT_CADENCE_USEC;
module_param (lines_per_frame, int, S_IRUGO | S_IWUSR);
module_param (points_per_line, int, S_IRUGO | S_IWUSR);
module_param (samples_per_point, int, S_IRUGO | S_IWUSR);
module_param (cadence_usec, int, S_IRUGO | S_IWUSR);

static int sample_adc=8;
static int sample_dac=8;
module_param (sample_adc, int, S_IRUGO | S_IWUSR);
module_param (sample_dac, int, S_IRUGO | S_IWUSR);

static int slot;
module_param (slot, int, S_IRUGO);

static char * pay_load=NULL;  /* area for free vectors from user space */
static int pay_load_size=0;

/* two mutex are used:
 * 'spm_device_available' blocks a second process trying to open the device; 
 * 'spm_device_in_use' blocks a second concurrent operation;
 * wait queue 'spm_dev_queue' is used to wait for the end of the operation
 * carried on in the interrupt system.
 * The 'irq_lock' spinlock protects the data shared between read/write
 * function and the interrupt handler.
 */

DECLARE_MUTEX(spm_device_available);
DECLARE_MUTEX(spm_device_in_use);

DECLARE_WAIT_QUEUE_HEAD(scan_wait_queue);

static enum {Nop,Ready,Working} spm_device_action = Nop;
static dev_t device_number;
static LIST_HEAD(device_list);

/*
 * shared resources for irq service routine
 */

static spinlock_t irq_lock = SPIN_LOCK_UNLOCKED;
static int ready_to_read=0;
static struct irq_parameters irq = 
{
	.unibuf = NULL,
        .data = NULL,
	.action = Idle,
	.completion = Irq_None,
	.request = Again
};

/*
 *   Functionality devices: scan and info
 */

struct scan_device {
	struct cdev cdev;
	dev_t  devnumber;
} scan, info;



int scan_open       (struct inode *, struct file *);
int scan_release    (struct inode *, struct file *);
ssize_t scan_read   (struct file *, char __user *, size_t, loff_t *);
ssize_t scan_write  (struct file *, const char __user *, size_t, loff_t *);

ssize_t info_read   (struct file *, char __user *, size_t, loff_t *);

static struct file_operations scan_fops = 
{
	.open    = scan_open,
	.release = scan_release,
	.read    = scan_read,
	.write   = scan_write
};

static struct file_operations info_fops = 
{
	.read    = info_read
};

/*
 *   Functions for the interrupt machinery
 */

struct event_head * spm_irq_handler (struct event_head *);
void null_function (void);

struct spm_external_functions daq_ops =
{
	.spm_irq_handler = (void *) null_function,
	.second_write = (void *) null_function,
	.set_timer = (void *) null_function,
	.set_samples = (void *) null_function,
	.set_adc = (void *) null_function,
};

void (* user_feedback_code) (struct irq_parameters *) = (void *)null_function;

/*
 *   functions exported by spm to DAQ board and feedback modules
 */

struct spm_external_functions * register_daq_board (
        int , void *, void *, void *);
void unregister_daq_board (int );
int get_devnumber (void);
void release_devnumber (int);
int register_feedback_code(void *);
void unregister_feedback_code(void *);

struct spm_export_symbols spm_global =
{
	.register_daq_board = register_daq_board,
        .unregister_daq_board = unregister_daq_board,
        .get_devnumber = get_devnumber,
        .release_devnumber = release_devnumber,
        .irq_lock = &irq_lock,
        .irq = &irq,
        .register_feedback_code = register_feedback_code,
        .unregister_feedback_code = unregister_feedback_code,
};

/* 
 *    get a dev number from available range
 */

static DECLARE_MUTEX(device_lock);
static int8_t minor_mask[MINOR_COUNT];

int get_devnumber(void) {

	int minor;

	down(&device_lock);

	for ( minor=0 ; minor<MINOR_COUNT ; minor++ ) {
		if (minor_mask[minor] == 0) {
			minor_mask[minor] = 1;
			break;
		}
	}
	up(&device_lock);

	if (minor == MINOR_COUNT) {
		DOUBLE_LOG ("%s:%s - No more minors available\n", HERE);
		return -1;
	}
		
	return MKDEV(scan_major, minor);
}

void release_devnumber (int devnumber) {

	down(&device_lock);
	minor_mask[MINOR(devnumber)] = 0;
	up(&device_lock);
}

#include "spm_dev_init_exit.c"
#include "spm_dev_daq_boards.c"
#include "scan_open_release.c"
#include "scan_read_write.c"
#include "info_read.c"

EXPORT_SYMBOL(spm_global);
