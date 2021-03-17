#!/usr/bin/bash

sudo rmmod drawpad_driver_intf_0 drawpad_driver_intf_1

cd interface_0
make
cd ..
cd interface_1
make
cd ..

sudo insmod ./interface_0/src/drawpad_driver_intf_0.ko
sudo insmod ./interface_1/src/drawpad_driver_intf_1.ko

