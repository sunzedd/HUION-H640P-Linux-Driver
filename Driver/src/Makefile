CONFIG_MODULE_SIG=n

obj-m += tablet_driver.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	find . -name '.tmp*' -delete

test:
	sudo dmesg -C
	sudo insmod tablet_driver.ko
