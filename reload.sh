#!/usr/bin/bash

sudo rmmod huion_drawpad
sudo dmesg -C
sudo insmod huion_drawpad.ko

