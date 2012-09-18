LINUX ?= /usr/src/linux

DIR=$(shell pwd)
TOPDIR=$(DIR:/spm_fc=)
FEEDBACK=$(TOPDIR)/feedback
FEEDBACK_CODES= $(basename $(shell cd $(FEEDBACK) && ls *.c))

all:	$(FEEDBACK_CODES)

$(FEEDBACK_CODES):
	mkdir -p module/$@
	ln -sf $(DIR)/kbuild module/$@/Makefile
	ln -sf $(DIR)/spm_fc.c module/$@/spm_fc.c
	ln -sf $(FEEDBACK)/$@.c module/$@/feedback_code.c
	echo \#define ALGORITHM_NAME \"$@\" > module/$@/algorithm_name.h
	cp ../spm_dev/Module.symvers module/$@/
	make -C `pwd`/module/$@ EXTRA_CFLAGS=-I$(TOPDIR)/include

clean:
	rm -rf module/*
	rm -f *~
