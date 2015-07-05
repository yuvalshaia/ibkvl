#obj-m := ibkvl.o

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

modules_install: default
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

clean:
	rm -f modules.order Module.symvers
	rm -f ibkvl.mod.c ibkvl.ko.unsigned *.o ibkvl.ko
