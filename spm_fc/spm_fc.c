/*
 *    spm_fc  - An Open Source package for digital feedback in 
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

#include "spm_bac.h"
#include "spm_dev_data.h"

#define NAME "spm_fc"

#include "algorithm_name.h"

static char algorithm_name[16] = ALGORITHM_NAME;
static char * _algorithm = algorithm_name;
module_param (_algorithm, charp, S_IRUGO);

/*
static int _start_x, _start_y;
module_param (_start_x, int, S_IRUGO | S_IWUSR);
module_param (_start_y, int, S_IRUGO | S_IWUSR);

static int _now_x, _now_y;
module_param (_now_x, int, S_IRUGO);
module_param (_now_y, int, S_IRUGO);

static int _span_x, _span_y;
module_param (_span_x, int, S_IRUGO | S_IWUSR);
module_param (_span_y, int, S_IRUGO | S_IWUSR);
*/

#include "feedback_code.c"

extern struct spm_export_symbols spm_global;

/*
 *      spm_fc_init - module initialization
 */

static int spm_fc_init (void) {

	int status = 0;

        spm_global.register_feedback_code(feedback_code);
        
        DOUBLE_LOG ("%s:%s - Loaded user feedback code \"%s\"\n",
                  HERE, _algorithm);

 	return status; 
}

/*
 *      spm_fc_exit - module removal
 */

static void spm_fc_exit (void) {

        spm_global.unregister_feedback_code(feedback_code);
        
        DOUBLE_LOG ("%s:%s - Unloaded user feedback code \"%s\"\n",
                  HERE, _algorithm);
}

module_init (spm_fc_init);
module_exit (spm_fc_exit);
