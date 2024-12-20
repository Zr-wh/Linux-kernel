EXTRA_CFLAGS += -g                                                                    
KVER = /usr/src/linux-5.0.2
CURDIR = $(shell pwd)
# Kernel modules
obj-m := pre_insmod.o
build: kernel_modules
kernel_modules:
	$(MAKE) -C $(KVER) M=$(CURDIR) modules
clean:
	$(MAKE) -C $(KVER) M=$(CURDIR) clean
 
