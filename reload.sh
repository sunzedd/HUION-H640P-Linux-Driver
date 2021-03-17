#!/usr/bin/bash

sudo rmmod drawpad_driver_intf_1

cd interface_1
make
cd ..

sudo insmod ./interface_1/src/drawpad_driver_intf_1.ko

