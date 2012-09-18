LINUX ?= /usr/src/linux

NIBAC_DEPS = nibac.c nibac.h nilib_dma.h \
	nibac_init_exit.c nibac_probe_remove.c nibac_init_exit.c \
	nibac_mmap.c nibac_interrupt_handler.c register_irq_handler.c\
	register_macro.h \
	../include/spm_dev_data.h \
	../include/spm_bac.h

NICONF_DEPS = niconf.c nilib.h nilib.c niscale.c acquire_board.c \
	nilib_dma.c nilib_dma.h

NIREGISTERS_DEPS = register_macro.h \
	tMSeries.constant.h \
	tMSeries.register_fields.h \
	tMSeries.register_names.h \
	tTIO.constant.h \
	tTIO.register_fields.h \
	tTIO.register_names.h

all:	$(NIBAC_DEPS) $(NIREGISTERS_DEPS)
	cp ../spm_dev/Module.symvers ./
	$(MAKE) -C $(LINUX) M=`pwd` modules
	$(MAKE) niconf

niconf:	$(NICONF_DEPS) $(NIREGISTERS_DEPS)
	gcc -Wall -I "../include" -o niconf \
	 niconf.c acquire_board.c nilib.c niscale.c nilib_dma.c

clean:
	rm -rf *.o *.ko *~ core .depend *.mod.c *.cmd .*.cmd .tmp_versions
	rm -f niconf modules.order Module.symvers

obj-m += nibac.o

