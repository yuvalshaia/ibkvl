KDIR ?= /lib/modules/`uname -r`/build

DRIVER_NAME = helloworld
TARGET = $(DRIVER_NAME).ko

#ccflags-y := -I$(src)/include

all: Module.symvers default

Module.symvers: ../ibkvl/Module.symvers
	cp $< $@

default: Module.symvers
	$(MAKE) -C $(KDIR) M=$$PWD

modules_install: default
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

clean:
	rm -rf $(TARGET) $(TARGET:.ko=.o) $(TARGET:.ko=.mod.c)
	rm -rf $(TARGET:.ko=.ko.unsigned) $(TARGET:.ko=.mod.o)
	rm -fr $(CFILES:.c=.o) .*cmd .tmp_versions
	rm -rf Module.markers Module.symvers modules.order
