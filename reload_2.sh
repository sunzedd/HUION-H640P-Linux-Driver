#!/usr/bin/bash

sudo rmmod drawpad_driver
sudo rmmod drawpad_driver_intf_0

sudo insmod ./src/drawpad_driver.ko
sudo insmod ./src_intf_0/drawpad_driver_intf_0.ko


