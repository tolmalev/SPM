
INCLUDE	= ../include/spm_dev_data.h \
          ../include/nibac/register_macro.h

NIBAC_H	= ../include/nibac.h ../include/nibac/*

FRAME_H	= frame.h ../include/spm_types.h ../include/spm_dev_data.h

CCFLAGS = -Wall -I "../include"

USER_DIR := $(shell pwd)
SPM_DIR := $(USER_DIR:/user=)

all:	frame latim genevent calibration_test

frame:	$(FRAME_H) frame.c frame_functions.c calibration.c calibration.h
	echo spm directory is $(SPM_DIR)
	gcc $(CCFLAGS) -DSPM_DIR=\"$(SPM_DIR)\" \
	frame.c frame_functions.c calibration.c -o frame

latim:	latim.c latim.h ../include/latim_pp_data.h latim_functions.c
	gcc $(CCFLAGS) latim.c -lm -o latim

genevent: genevent.c ../include/spm_dev_data.h
	gcc $(CCFLAGS) genevent.c -o genevent -lm

calibration_test: calibration.c calibration.h
	gcc $(CCFLAGS) -DCALIBRATION_TEST=1 calibration.c -o calibration_test

clean:	
	rm -f latim frame genevent calibration_test
	rm -f *~ data_*.scan log-*.dat log*.lst
