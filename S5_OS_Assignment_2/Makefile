obj-m += cdd_module.o

KVERSION = $(shell uname -r)
KDIR = /lib/modules/$(KVERSION)/build

all:
	make -C $(KDIR) M=$(shell pwd) modules

clean:
	make -C $(KDIR) M=$(shell pwd) clean