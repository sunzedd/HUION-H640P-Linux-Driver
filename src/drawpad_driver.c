#ifndef HUION_H640P_DRAWPAD_DRIVER
#define HUION_H640P_DRAWPAD_DRIVER

#include "log_utils.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>


#define DRIVER_NAME     "Huion H640P Driver"
#define DRIVER_AUTHOR   "Rostislav V."

#define VENDOR_ID     0x256c    // Huion Animation Co.
#define PRODUCT_ID    0x006d    // H640P Drawpad


/*                     Driver Entry Points Headers                   */
static int drawpad_probe(struct usb_interface *interface, 
                         const struct usb_device_id *dev_id);
static void drawpad_disconnect(struct usb_interface* interface);
/* ----------------------------------------------------------------- */

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

typedef
struct drawpad {
    char name[128];
    char phys[64];
    struct usb_device   *usb_dev;
    struct input_dev    *input_dev;
    struct urb          *irq;
    char *dma;
    dma_addr_t data_dma;
} drawpad_t;

static 
int drawpad_probe(struct usb_interface *interface, 
                  const struct usb_device_id *dev_id) {

    LOG_INFO("call drawpad_probe");

    // Получаем 'дескриптор' интерфейса
    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    
    LOG_INFO("probe device (%04x:%04X). Interface number %d\n",
              dev_id->idVendor, dev_id->idProduct,
              interface_desc->desc.bInterfaceNumber);
    
    LOG_INFO("\tEndpoints count: %d\n", interface_desc->desc.bNumEndpoints);

    for (int i = 0; i < interface_desc->desc.bNumEndpoints; i++) {
        struct usb_endpoint_descriptor *endpoint_desc = 
            &(interface_desc->endpoint[i].desc);

        LOG_INFO("\t\tEndpoint № %d -> bEndPointAddress: 0x%02X\n",
                  i, endpoint_desc->bEndpointAddress);
        LOG_INFO("\n");
    }

    return 0;
}

static 
void drawpad_disconnect(struct usb_interface* interface) {
    LOG_INFO("call drawpad_disconnect");
}

static 
int __init drawpad_init(void) {
    int rc = 0;
    if (0 == (rc = usb_register(&drawpad_driver))) {
        LOG_INFO("call usb_register: OK\n");
    } else {
        LOG_ERR("call usb_register: FAILED\n");
    }

    return rc;
}

static 
void __exit drawpad_exit(void) {
    LOG_INFO("call usb_deregister\n");
    usb_deregister(&drawpad_driver);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);

module_init(drawpad_init);
module_exit(drawpad_exit);

#endif // HUION_H640P_DRAWPAD_DRIVER