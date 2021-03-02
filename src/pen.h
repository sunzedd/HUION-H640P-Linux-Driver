#ifndef __HUION_DRAWPAD_PEN__
#define __HUION_DRAWPAD_PEN__

#include <linux/init.h>
#include "fetch_dev_info.h"

#define PEN_ENDPOINT_ADDRESS 0x81

static struct usb_class_driver *pen_class_driver;

static
int pen_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("call pen_probe\n");
    print_usb_interface_description(interface);

    // class.name = "usb/drawpad_pen%d";
    // class.fops = &pen_fops;
    // int rc = usb_register_dev(interface, &pen_class_driver);

    return 0;
}

static
void pen_disconnect(struct usb_interface *interface) {
    LOG_INFO("call pen_disconnect\n");
    // usb_deregister_dev(interface, &pen_class_driver);
}

#endif // __HUION_DRAWPAD_PEN__
