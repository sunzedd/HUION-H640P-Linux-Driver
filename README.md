# HUION-H640P-Linux-Driver

## Operating Systems Course Project 2020

Before plugin the HUION tablet you should remove usbhid
kernel module to prevent handling the tablet by defaut usbhid driver:

```
sudo rmmod usbhid
```

or it can be done permanently on current machine by modifying /etc/default/grub file:

```
GRUB_CMDLINE_LINUX_DEFAULT="usbhid.quirks=0x256c:0x006d:0x4"
```

do update-grub command after this.


