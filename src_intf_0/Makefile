CONFIG_MODULE_SIG=n
ccflags-y := -std=gnu99 -Wno-declaration-after-statement -ffreestanding

obj-m = drawpad_driver_intf_0.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
