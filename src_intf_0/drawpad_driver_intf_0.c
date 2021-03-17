#ifndef __DRAWPAD_DRIVER_INTF_0__
#define __DRAWPAD_DRIVER_INTF_0__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <linux/interrupt.h>

#include "../log_utils.h"


#define DRIVER_NAME     "Huion H640P Driver Interface 0"
#define DRIVER_AUTHOR   "Rostislav V."

#define VENDOR_ID       0x256c    // Huion Animation Co.
#define PRODUCT_ID      0x006d    // H640P Drawpad


static int drawpad_probe(struct usb_interface *interface,
                         const struct usb_device_id *dev_id);

static int pad_probe(struct usb_interface *interface,
                     const struct usb_device_id *dev_id);

static void drawpad_disconnect(struct usb_interface* interface);
static void pad_disconnect(struct usb_interface* interface);


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
    int rc = -1;

    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    LOG_INFO_PEN("probe device (%04x:%04X) Interface: %d\n", dev_id->idVendor, dev_id->idProduct, interface_number);

    if (interface_number == 0)
        rc = pad_probe(interface, dev_id);
   
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

struct pad {
    char    phys[32];
    struct usb_device   *usb_device;
    struct input_dev    *input_device;
    struct urb      *urb;
    unsigned char   *transfer_buffer;
    unsigned int    transfer_buffer_size;
    dma_addr_t      dma;

    uint8_t pen_touchdown;
    uint8_t pen_above_pad; 
};

static struct pad *pad;

static void tasklet_handler(unsigned long tasklet_data) {
    
    LOG_INFO_PEN("TASKLET HANDLER\n");
}

DECLARE_TASKLET(pad_tasklet, tasklet_handler, 0);


static
void pad_irq(struct urb *urb) {
    int rc = 0;

    struct pad *pad = urb->context;

    if (urb->status == 0) {
        
        tasklet_schedule(&pad_tasklet);

        rc = usb_submit_urb(pad->urb, GFP_ATOMIC);
        if (rc) {
            LOG_ERR_PEN("\tusb_submit_urb failed\n");
        }

    } else {
        LOG_ERR_PEN("\tError urb status recieved: ");
        switch (urb->status) {
            case -ENOENT: LOG_ERR_PEN("\t\tENOENT (killed by usb_kill_urb)\n"); break;
            case -ECONNRESET: LOG_ERR_PEN("\t\tECONNRESET\n"); break;
            case -EINPROGRESS: LOG_ERR_PEN("\t\tEINPROGRESS\n"); break;
            default: LOG_ERR_PEN("\t\tanother error: %d\n", urb->status); break;
        }
    }
}

static
int pad_open(struct input_dev* input_device) {

    struct pad *pad = input_get_drvdata(input_device);

    if (usb_submit_urb(pad->urb, GFP_KERNEL)) {
        return -EIO;
    }

    return 0;
}

static
void pad_close(struct input_dev* input_device) {
    struct pad *pad = input_get_drvdata(input_device);
    usb_kill_urb(pad->urb);
}

static
int pad_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    int rc = -ENOMEM;

    struct usb_endpoint_descriptor *endpoint = 
        &interface->cur_altsetting->endpoint[0].desc;

    pad = kzalloc(sizeof(struct pad), GFP_KERNEL);
    if (!pad) {
        LOG_ERR_PEN("\tstruct pad allocation FAILURE\n");
        return rc;
    }

    pad->usb_device = interface_to_usbdev(interface);
    pad->input_device = input_allocate_device();
    if (!pad->input_device) {
        LOG_ERR_PEN("\tinput_allocate_device FAILURE\n");
        kfree(pad);
        return rc;
    }

    pad->transfer_buffer_size = endpoint->wMaxPacketSize;
    pad->transfer_buffer = usb_alloc_coherent(pad->usb_device,
                                              pad->transfer_buffer_size,
                                              GFP_KERNEL, &pad->dma);
    if (!pad->transfer_buffer) {
        LOG_ERR_PEN("\ttransfer buffer allocation FAILURE\n");
        input_free_device(pad->input_device);
        kfree(pad);
        return rc;
    }

    usb_make_path(pad->usb_device, pad->phys, sizeof(pad->phys));
    strlcat(pad->phys, "/input0", sizeof(pad->phys));
    LOG_INFO_PEN("usb_path: %s\n", pad->phys);


    pad->input_device->name = "Huion H640P Interface 0";
    pad->input_device->phys = pad->phys;
    usb_to_input_id(pad->usb_device, &pad->input_device->id);
    pad->input_device->dev.parent = &interface->dev;

    input_set_drvdata(pad->input_device, pad);

    pad->input_device->open = pad_open;
    pad->input_device->close = pad_close;

    pad->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!pad->urb) {
        LOG_ERR_PEN("\tusb_alloc_urb FAILURE\n");
        input_free_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
        return rc;
    }

    pad->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    int urb_pipe = usb_rcvintpipe(pad->usb_device, endpoint->bEndpointAddress);
    LOG_INFO_PEN("pipe: %d, endpoint: %x\n", urb_pipe, endpoint->bEndpointAddress);

    usb_fill_int_urb(pad->urb, pad->usb_device, urb_pipe,
                     pad->transfer_buffer, pad->transfer_buffer_size,
                     pad_irq, pad,
                     endpoint->bInterval);
    pad->urb->transfer_dma = pad->dma;
    
    rc = input_register_device(pad->input_device);
    if (rc == 0) {
        usb_set_intfdata(interface, pad);
    } else {
        LOG_ERR_PEN("\tinput_register_device FAILURE\n");
        usb_free_urb(pad->urb);
        input_free_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
    }

    return rc;
}

static
void pad_disconnect(struct usb_interface *interface) {
    
    struct pad *pad = usb_get_intfdata(interface);
    if (pad) {
        usb_kill_urb(pad->urb);
        usb_free_urb(pad->urb);
        input_unregister_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
    }
}

#endif // __DRAWPAD_DRIVER_INTF_0__