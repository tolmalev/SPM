LINUX ?= /usr/src/linux

INCLUDE	= ../include/spm_dev_data.h ../include/spm_bac.h

SPM_DEV = spm_dev.c \
	  spm_dev_init_exit.c \
	  scan_open_release.c \
	  scan_read_write.c \
	  spm_dev_daq_boards.c

all:	$(SPM_DEV) $(INCLUDE)
	$(MAKE) -C $(LINUX) M=`pwd` modules

#install:
#	$(MAKE) -C $(LINUX) M=`pwd` modules_install

clean:
	$(MAKE) -C $(LINUX) M=`pwd` clean
	rm -f *~ modules.order Module.symvers

obj-m += spm_dev.o
