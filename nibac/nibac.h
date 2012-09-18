/*
 *      data structure specific to National Instruments DAQ boards
 *
 */

#include "tMite.h"
#include "register_macro.h"

/*
 *   data structures and functions for pci devices (DAQ boards) management
 */

static int nibac_probe_device (struct pci_dev *, const struct pci_device_id *);
static void nibac_remove_device (struct pci_dev *);


#define NAME_SIZE 16
#define NUMBER_OF_BARS  2

struct nibac_pci_device {
	struct list_head link;              /* double linked list */
	struct cdev cdev;
	struct pci_dev *pci_device;         /* linux pci device object */
	char   name[NAME_SIZE];
	u8 irq_line;
	int rank;
	dev_t devnumber;
	int bar[NUMBER_OF_BARS];             /* pci device resources */
	int memory_region[NUMBER_OF_BARS];
	struct memaps {
		u8 * registers;
		u8 * shadow;
	} memap[NUMBER_OF_BARS];
        u8 * calibration;
	struct spm_external_functions * daq_ops;
        dma_addr_t dma_phys_addr;            /* board address of DMA area */
};

/*
 *    file operations
 */

static int nibac_open    (struct inode *, struct file *);
static int nibac_release (struct inode *, struct file *);
static int nibac_mmap  (struct file *, struct vm_area_struct *);
static ssize_t nibac_write (struct file *, const char __user *,
			    size_t, loff_t *);
static ssize_t nibac_read (struct file *, char __user *, size_t, loff_t *);

static int nibac_irq_handler (int , void * , struct pt_regs *);
static void nibac_set_timer (int);
static void nibac_second_write (struct event_head *, int);
static void release_resources (struct nibac_pci_device *device);

/*
 * board register access
 */

//u8  ** mwm=NULL, ** mwr=NULL, ** swm=NULL, ** swr=NULL;

extern struct spm_export_symbols spm_global;
