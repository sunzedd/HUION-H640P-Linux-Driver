#ifndef HUION_H640P_DRAWPAD_DRIVER
#define HUION_H640P_DRAWPAD_DRIVER

#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "log_utils.h"


#define DRIVER_NAME     "Huion H640P Driver"
#define DRIVER_AUTHOR   "Rostislav V."

#define VENDOR_ID     0x256c    // Huion Animation Co.
#define PRODUCT_ID    0x006d    // H640P Drawpad


/*                     Driver Entry Points Headers                   */
static int drawpad_probe(struct usb_interface *interface, 
                         const struct usb_device_id *dev_id);
static void drawpad_disconnect(struct usb_interface* interface);
/* ----------------------------------------------------------------- */


/*                         2 Level functions                                 */
void print_usb_interface_description(const struct usb_interface *interface);
int get_drawpad_interface_type(const struct usb_interface *interface);
/*---------------------------------------------------------------------------*/


enum DRAWPAD_INTERFACE {
    PEN = 0,
    PAD = 1
};


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

    LOG_INFO("probe device (%04x:%04X)", dev_id->idVendor, dev_id->idProduct);
    
    // определить, какой именно интерфейс вызвал функцию probe? (планшет или стилус)
    int drawpad_interface_type = get_drawpad_interface_type(interface);
    switch (drawpad_interface_type) {
    case PEN:
        // pen_probe();
        LOG_INFO("Drawpad Interface Type: PEN\n");
        break;
    case PAD:
        // pad_probe();
        LOG_INFO("Drawpad Interface Type: PAD\n");
        break;
    }

    print_usb_interface_description(interface);
    
    return 0;
}

static 
void drawpad_disconnect(struct usb_interface* interface) {
    LOG_INFO("call drawpad_disconnect");

    // дерегистрация соответствующего USB устройства.
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


/*                       2 level functions                                  */
void print_usb_interface_description(const struct usb_interface *interface) {

    struct usb_host_interface *interface_desc = interface->cur_altsetting;

    LOG_INFO("\tInterface number: %d\n", interface_desc->desc.bInterfaceNumber);
    LOG_INFO("\tEndpoints count: %d\n", interface_desc->desc.bNumEndpoints);

    for (int i = 0; i < interface_desc->desc.bNumEndpoints; i++) {

        struct usb_endpoint_descriptor *endpoint_desc = 
            &(interface_desc->endpoint[i].desc);

        LOG_INFO("\t\tEndpointDescriptor[%d] -> bEndPointAddress: 0x%02X\n\n",
                  i, endpoint_desc->bEndpointAddress);
    }
}

int get_drawpad_interface_type(const struct usb_interface *interface) {
    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    
    int drawpad_interface_type = -1;
    
    switch (interface_desc->desc.bInterfaceNumber) {
        case 0: drawpad_interface_type = PEN; break;
        case 1: drawpad_interface_type = PAD; break;
        // handle default?
    }

    return drawpad_interface_type;
}

/*---------------------------------------------------------------------------*/


#endif // HUION_H640P_DRAWPAD_DRIVER