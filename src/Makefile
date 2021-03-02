CONFIG_MODULE_SIG=n

ccflags-y := -std=gnu99 -Wno-declaration-after-statement
obj-m += drawpad_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	find . -name '.tmp*' -delete
