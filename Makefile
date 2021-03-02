CONFIG_MODULE_SIG=n
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

obj-m = huion_drawpad.o
huion_drawpad-objs := ./src/fetch_dev_info.o ./src/huion_drawpad.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
