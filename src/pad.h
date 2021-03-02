#ifndef __HUION_DRAWPAD_PAD__
#define __HUION_DRAWPAD_PAD__

#include <linux/init.h>
#include "fetch_dev_info.h"

#define PAD_ENDPOINT_ADDRESS 0x82

static struct usb_class_driver *pad_class_driver;

static
int pad_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("call pad_probe\n");    
    print_usb_interface_description(interface);

    return 0;
}

static
void pad_disconnect(struct usb_interface *interface) {
    LOG_INFO("call pad_disconnect\n");
}

#endif // __HUION_DRAWPAD_PAD__