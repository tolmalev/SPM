
all:
	make -C spm_dev
	make -C nobac
	make -C nibac
	make -C iobac
	make -C mcbac
	make -C user
	make -C latim_pp
	make -C spm_fc

clean:
	make -C spm_dev clean
	make -C nibac clean
	make -C iobac clean
	make -C mcbac clean
	make -C nobac clean
	make -C user clean
	make -C latim_pp clean
	make -C spm_fc clean
	rm -f *~
	rm -f feedback/*~
	rm -f include/*~
