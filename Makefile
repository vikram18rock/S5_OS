obj-m += character_device_driver.o

KVERSION = $(shell uname -r)
KDIR = /lib/modules/$(KVERSION)/build

all:
	make -C $(KDIR) M=$(shell pwd) modules

clean:
	make -C $(KDIR) M=$(shell pwd) clean