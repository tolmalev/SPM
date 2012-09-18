#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include  <errno.h>

#include "spm_types.h"
#include "register_macro.h"

#define DEVICE "/dev/spm/nibac0"
#define NAM "acquire_board"

#define s16 i16

#include "nilib.h"
#include "spm_dev_data.h"

/*
 * open the device and map register area(s)
 */

u8 ** acquire_board (char * device, /* device name (def. /dev/nibac0" */
		     int D,         /* debug level */
		     int * fd       /* device file descriptor */) {

	char * dev;
	u8 * mem0, * mem1;
	pid_t mypid;

	u8 ** memap;

	if (D>2) printf ("%s - starting\n", NAM);

	/*
	 * allocate memory for memap
	 *
         *    memap[0]: bar 1 address (function registers area)
         *    memap[1]: bar 1 shadow area
         *    memap[2]: bar 0 address (Mite registers area)
         *    memap[3]: bar 0 shadow area for calibration constants
         *    memap[4]: calibration constants area
         *
	 */

	memap = malloc(sizeof(memap) * 5);
	if (!memap) {
		printf ("%s - malloc failed\n", NAM);
		exit (-1);
	}

	/*
	 *   open the nibac device
	 */

	if (device) dev=device;
	else {
		printf ("%s - device name not given; using default %s\n",
			NAM, DEVICE);
		dev=DEVICE;
	}
  
	printf ("%s: configuring board %s\n", NAM, dev);
  
	*fd = open (dev, O_RDWR);
	if ( *fd <= 0 ) {
                free (memap);
		perror (NAM);
		return NULL;
	}

	/*
	 *  mmap the board windows - area codes are as follows:
	 *
	 *  4096 * ( NIBAC_MMAP_<what> + <bar number (0/1)> )
	 *
	 *  see nibac_data.h
	 *
	 *  bar0 - the MITE chip registers
	 */

	mem0 = mmap (NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 
		     0 * 4096);

	if (D>1) printf ("%s - bar0 mapped at %p\n", NAM, mem0);

	if (mem0 == MAP_FAILED) {
		printf ("%s - Mapping of bar0 failed!\n", NAM);
		close (*fd);
		exit (-1);
	}

	printf ("%s - model/vendor 0x%4.4x 0x%4.4x\n",
		NAM, read16(mem0,0x302), read16(mem0, 0x300));

	if (D>=2) {
		printf ("\n%s - content of register bank #0\n\n", NAM);
		dump_memory (mem0,4096);
	}

	memap[2] = mem0;

	/*
	 *    bar1 - the DAQ registers window.
	 */

	mem1 = mmap (NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, *fd,
		     1 * 4096);

	if (D>1) printf ("%s - bar1 mapped at %p\n", NAM, mem1);

	if (mem1 == MAP_FAILED) {
		printf ("%s - Mapping of bar1 failed!\n", NAM);
		close (*fd);
		exit (-1);
	}

	memap[0] = mem1;

	if (D>=2) {
		printf ("\n%s - content of register bank #1\n\n", NAM);
		dump_memory (mem1,4096);
	}

	/*
	 * request m'mapping shadow register area for bar 1
	 *
	 */

	memap[1] = mmap (NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, *fd,
			 (MMAP_REGIONS + 1) * 4096);

	if (D>1) printf ("%s - shadow area for bar1 mapped at %p\n",
			 NAM, memap[1]);

	if (memap[1] == MAP_FAILED) {
                printf ("%s - Mapping of bar1 shadow failed!\n", NAM);
		close (*fd);
		exit (-1);
	}
	if (memap[1]) {
		if (D>1) printf ("%s - Area content: %d %d %d %d %d ...\n",
				 NAM, memap[1][0], memap[1][1], memap[1][2],
				 memap[1][3], memap[1][4]);
	} else {
		if (D>1) printf ("%s - shadow register mapping failed\n", NAM);
	}

	if (D>=2) {
		printf ("\n%s - content of shadow register bank #1: 0 --> "
			"%x\n\n",NAM,(int)sizeof(struct shadow_M_registers)); 
		dump_memory (memap[1], sizeof(struct shadow_M_registers));
	}

	/*
	 * request m'mapping shadow register area for bar 0
	 *
	 */

	memap[3] = mmap (NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, *fd,
			 (MMAP_REGIONS + 0) * 4096);

	if (D>1) printf ("%s - shadow area for bar0 mapped at %p\n",
			 NAM, memap[3]);

	if (memap[3] == MAP_FAILED) {
                printf ("%s - Mapping of bar0 shadow failed!\n", NAM);
		close (*fd);
		exit (-1);
	}
	if (memap[3]) {
		if (D>1) printf ("%s - Area content: %d %d %d %d %d ...\n",
				 NAM, memap[3][0], memap[3][1], memap[3][2],
				 memap[3][3], memap[3][4]);
	} else {
		if (D>1) printf ("%s - shadow register mapping failed\n", NAM);
	}

	if (D>=2) {
		printf ("\n%s - content of shadow register bank #0: 0 --> "
			"%x\n\n",NAM,(int)sizeof(struct shadow_M_registers)); 
		dump_memory (memap[3], sizeof(struct shadow_M_registers));
	}

	/*
	 *
	 * mmap area for calibration array
	 *
	 */

	memap[4] = mmap (NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, *fd,
                         (2 * MMAP_REGIONS + 1) * 4096);

	if (D>1) printf ("%s - area for calibrations mapped at %p\n",
			 NAM, memap[4]);

	if (memap[4] == MAP_FAILED) {
		printf ("%s - Mapping of calibration area failed!\n", NAM);
		close (*fd);
		exit (-1);
	}

	mypid = getpid();
	if (D>2) printf ("\n%s: my pid is %d\n", NAM, mypid);

	return memap;
}

/*
 *   dump of a memory bank
 */

void dump_memory (u8 * area, u32 size) {

#define lsize 16

	int i, j, line, do_print;

	line = 0;
	for ( i=0 ; i<size ; i+=lsize ) {
		if (i) {
			do_print = 0;
			for ( j=0 ; j<lsize ; j+=4)
				do_print |= (read32(area,j+line) !=
					     read32(area,j+i));
		} else do_print = 1;
		if (do_print) {
			printf (" %6x   ", i);
			for ( j=0 ; j<lsize && i+j < size ; j+=4) {
				printf ("  %8.8x", read32(area,j+i));
			}
			printf ("\n");
			line = i;
		} else if (i == line+lsize) printf ("     ...\n");
	}
	return;
}
