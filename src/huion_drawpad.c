#ifndef __HUION_DRAWPAD__
#define __HUION_DRAWPAD__

#include <linux/module.h>
#include "pad.h"


#define DRIVER_NAME     "Huion H640P Driver"
#define DRIVER_AUTHOR   "Rostislav V."

#define VENDOR_ID     0x256c    // Huion Animation Co.
#define PRODUCT_ID    0x006d    // H640P Drawpad


/*                     Driver Entry Points Headers                   */
static int drawpad_probe(struct usb_interface *interface, 
                         const struct usb_device_id *dev_id);
static void drawpad_disconnect(struct usb_interface* interface);


static 
struct usb_device_id devices_table[] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};

MODULE_DEVICE_TABLE(usb, devices_table);

static 
struct usb_driver drawpad_driver = {
    .name = DRIVER_NAME,
    .id_table = devices_table,
    .probe = drawpad_probe,
    .disconnect = drawpad_disconnect,
};


static 
int drawpad_probe(struct usb_interface *interface, 
                  const struct usb_device_id *dev_id) {

    LOG_INFO("probe device (%04x:%04X)\n", dev_id->idVendor, dev_id->idProduct);
    
    int rc = -1;

    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    if (interface_number == 1) {
        rc = pad_probe(interface, dev_id);
    }
   
    return rc;
}

static 
void drawpad_disconnect(struct usb_interface* interface) {
    LOG_INFO("disconnect device\n");
    
    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    if (interface_number == 1) {
        pad_disconnect(interface);
    }
}

static 
int __init drawpad_init(void) {

    int rc = usb_register(&drawpad_driver);
    if (rc != 0) {
        LOG_ERR("call usb_register: FAILED\n");
    }

    return rc;  
}

static 
void __exit drawpad_exit(void) {
    usb_deregister(&drawpad_driver);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);

module_init(drawpad_init);
module_exit(drawpad_exit);

#endif // __HUION_DRAWPAD__