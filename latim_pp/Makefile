LINUX ?= /usr/src/linux

LATIM = latim_pp.c ../include/latim_pp_data.h latim_pp_init_exit.c \
	latim_pp_interrupt_handler.c latim_pp_open_release.c \
	latim_pp_read_write.c

all:	$(LATIM)
	$(MAKE) -C $(LINUX) SUBDIRS=`pwd` modules

clean:
	rm -rf *.o *.ko *~ core .depend *.mod.c *.cmd .*.cmd .tmp_versions \
	Module.symvers modules.order

obj-m += latim_pp.o
