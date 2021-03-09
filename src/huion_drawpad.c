#ifndef __HUION_DRAWPAD__
#define __HUION_DRAWPAD__

#include <linux/module.h>

#include "fetch_dev_info.h"
#include "pad.h"
//#include "pen.h"


#define DRIVER_NAME     "Huion H640P Driver"
#define DRIVER_AUTHOR   "Rostislav V."

#define VENDOR_ID     0x256c    // Huion Animation Co.
#define PRODUCT_ID    0x006d    // H640P Drawpad


typedef 
struct {
    struct usb_interface *pen_interface;
    struct usb_interface *pad_interface;
    struct usb_device *device;
} drawpad_t;

static drawpad_t drawpad_object;


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
    .probe = drawpad_probe,                     // probe - driver entry point
    .disconnect = drawpad_disconnect,           // disconnect - entry point 2
};


static 
int drawpad_probe(struct usb_interface *interface, 
                  const struct usb_device_id *dev_id) {

    LOG_INFO("probe device (%04x:%04X)", dev_id->idVendor, dev_id->idProduct);
    
    int rc = 0;

    if (drawpad_object.device == NULL) {
        drawpad_object.device = interface_to_usbdev(interface);
    }

    switch (get_drawpad_interface_type(interface)) {
        case PEN:
            //drawpad_object.pen_interface = interface;
            //rc = pen_probe(interface, dev_id);
            break;

        case PAD:
            drawpad_object.pad_interface = interface;
            rc = pad_probe(interface, dev_id);
            break;
    }
    
    LOG_INFO("\n");

    return rc;
}

static 
void drawpad_disconnect(struct usb_interface* interface) {

    LOG_INFO("call drawpad_disconnect");

    switch (get_drawpad_interface_type(interface)) {
       // case PEN: pen_disconnect(interface); break;
        case PAD: pad_disconnect(interface); break;
    }
}

static 
int __init drawpad_init(void) {

    int rc = 0;

    if (0 == (rc = usb_register(&drawpad_driver))) {    // регистрация ДРАЙВЕРА в подстистеме USB.
        LOG_INFO("call usb_register: OK\n");
    } else {
        LOG_ERR("call usb_register: FAILED\n");
    }

    return rc;  
}

static 
void __exit drawpad_exit(void) {
    LOG_INFO("call usb_deregister\n"); // дерегистрация ДРАЙВЕРА в подстистеме USB.
    usb_deregister(&drawpad_driver);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);

module_init(drawpad_init);
module_exit(drawpad_exit);

#endif // __HUION_DRAWPAD__