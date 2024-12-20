                                                             
ifneq  ($(KERNELRELEASE),)
#kbuild part of makefile
KBUILD_EXTRA_SYMBOLS+=/home/localhost/wh/ko_test/after_insmod/Module.symvers

obj-m:=after_insmod.o
$(info "2nd")
else
KDIR := /usr/src/linux-5.0.2
PWD:=$(shell pwd)
all:
	$(info "1st")
	make -C  $(KDIR)  M=$(PWD) modules
clean:
	rm -f *.ko *.o *.symvers *.mod.c *.mod.o *.order
endif
