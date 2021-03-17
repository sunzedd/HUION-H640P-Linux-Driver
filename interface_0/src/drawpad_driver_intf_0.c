#ifndef __DRAWPAD_DRIVER__
#define __DRAWPAD_DRIVER__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <linux/interrupt.h>

#include "log_utils.h"


#define DRIVER_NAME     "Huion H640P Interface 0 Driver"
#define DRIVER_AUTHOR   "Rostislav V."

#define VENDOR_ID       0x256c    // Huion Animation Co.
#define PRODUCT_ID      0x006d    // H640P Drawpad


static int probe(struct usb_interface *interface, 
                 const struct usb_device_id *dev_id);

static int probe_interface_0(struct usb_interface *interface,
                             const struct usb_device_id *dev_id);

static void disconnect(struct usb_interface* interface);
static void disconnect_interface_0(struct usb_interface* interface);


static struct usb_device_id devices_table[] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};

MODULE_DEVICE_TABLE(usb, devices_table);

static struct usb_driver drawpad_driver = {
    .name = DRIVER_NAME,
    .id_table = devices_table,
    .probe = probe,
    .disconnect = disconnect,
};

static int probe(struct usb_interface *interface, const struct usb_device_id *dev_id) {
    int rc = -1;

    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    LOG_INFO_INTF_0("probe device (%04x:%04X) Interface: %d\n",
                    dev_id->idVendor, dev_id->idProduct, interface_number);

    if (interface_number == 0) {
        rc = probe_interface_0(interface, dev_id);
    }
   
    return rc;
}

static void disconnect(struct usb_interface* interface) {
    printk(KERN_INFO "disconnect device\n");
    
    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    if (interface_number == 0) {
        disconnect_interface_0(interface);
    }
}

static int __init drawpad_driver_init(void) {

    int rc = usb_register(&drawpad_driver);
    if (rc != 0) {
        printk(KERN_ERR "call usb_register: FAILED\n");
    }

    return rc;  
}

static void __exit drawpad_driver_exit(void) {
    usb_deregister(&drawpad_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);

module_init(drawpad_driver_init);
module_exit(drawpad_driver_exit);


struct drawpad {
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

static struct drawpad *pad;

static void pad_init_pen_status(struct drawpad *pad) {
    pad->pen_touchdown = 0;
    pad->pen_above_pad = 0;
}

static const int input_event_types[] = {

};


static void tasklet_handler(unsigned long tasklet_data) {
    /*
    uint8_t header;
    uint8_t pen_status;
    uint16_t x; 
    uint16_t y; 
    uint16_t pressure; 

    unsigned char *data = pad->transfer_buffer;

    header = data[0];
    pen_status = data[1];
    memcpy(&x, &data[2], 2);
    memcpy(&y, &data[4], 2);
    memcpy(&pressure, &data[6], 2);

    
    if (header != 0xa) {
        LOG_ERR_INTF_0("Invalid packet recieved. Header = %x\n", header);
        return;
    }

    if (!pad->pen_above_pad) {
        if(pen_status & 0xc0) {
            pad->pen_above_pad = 1;
            input_report_key(pad->input_device, BTN_TOOL_PEN, 1);
        }

    } else {
        if (!(pen_status & 0xc0)) {
            pad->pen_above_pad = 0;
            input_report_key(pad->input_device, BTN_TOOL_PEN, 0);
        }
    }

    if (!pad->pen_touchdown) {
        if (pen_status & 0x1) {
            pad->pen_touchdown = 1;
            input_report_key(pad->input_device, BTN_TOUCH, 1);
        }

    } else {
        if (!(pen_status & 0x1)) {
            pad->pen_touchdown = 0;
            input_report_key(pad->input_device, BTN_TOUCH, 0);
        }
    }

    if (pen_status & 0x2) {
        input_report_key(pad->input_device, BTN_STYLUS, 1);
    } else {
        input_report_key(pad->input_device, BTN_STYLUS, 0);
    }

    if (pen_status & 0x4) {
        input_report_key(pad->input_device, BTN_STYLUS2, 1);
    } else {
        input_report_key(pad->input_device, BTN_STYLUS2, 0);
    }

    if (pad->pen_above_pad || pad->pen_touchdown) {
        input_report_abs(pad->input_device, ABS_X, x * X_FACTOR);
        input_report_abs(pad->input_device, ABS_Y, y * Y_FACTOR);
        input_report_abs(pad->input_device, ABS_PRESSURE, pressure);
    }

    LOG_INFO_INTF_0("head: %x, pen: %x, x: %hu, y: %hu, press: %hu\n",
                     header, pen_status, x * X_FACTOR, y * Y_FACTOR, pressure);

    input_sync(pad->input_device);
    */
    LOG_INFO_INTF_0("TASKLET RUN!\n");
}

DECLARE_TASKLET(drawpad_tasklet_interface_0, tasklet_handler, 0);


static void pad_irq(struct urb *urb) {
    struct drawpad *pad = urb->context;

    if (urb->status == 0) {

        tasklet_schedule(&drawpad_tasklet_interface_0);

        int rc = usb_submit_urb(pad->urb, GFP_ATOMIC);
        if (rc) {
            LOG_ERR_INTF_0("\tfailed to submit urb\n");
        } else { 
            LOG_INFO_INTF_0("submited urb\n");
        }

    } else {
        LOG_WARN_INTF_0("\twarning: urb status recieved: ");

        switch (urb->status) {
            case -ENOENT:
                LOG_ERR_INTF_0("\t\tENOENT (killed by usb_kill_urb)\n");
                break;

            default:
                LOG_ERR_INTF_0("\t\tanother error: %d\n", urb->status);
                break;
        }
    }
}

static int open_interface_0(struct input_dev* input_device) {

    struct drawpad *pad = input_get_drvdata(input_device);
    LOG_INFO_INTF_0("open\n");

    int rc = usb_submit_urb(pad->urb, GFP_KERNEL);
    if (rc) {
        LOG_ERR_INTF_0("Failed to submit urb, code: %d", rc);
        return -EIO;
    } else {
        LOG_INFO_INTF_0("submited urb\n");
    }

    return 0;
}

static void close_interface_0(struct input_dev* input_device) {
    LOG_INFO_INTF_0("close\n");
    struct drawpad *pad = input_get_drvdata(input_device);
    usb_kill_urb(pad->urb);
    //usb_unlink_urb(pad->urb);
}

static int probe_interface_0(struct usb_interface *interface,
                             const struct usb_device_id *dev_id) {
    int rc = -ENOMEM;

    struct usb_endpoint_descriptor *endpoint = 
        &interface->cur_altsetting->endpoint[0].desc;

    pad = kzalloc(sizeof(struct drawpad), GFP_KERNEL);
    if (!pad) {
        LOG_ERR_INTF_0("\tstruct drawpad allocation FAILURE\n");
        return rc;
    }

    pad->usb_device = interface_to_usbdev(interface);
    pad->input_device = input_allocate_device();
    if (!pad->input_device) {
        LOG_ERR_INTF_0("\tinput_allocate_device FAILURE\n");
        kfree(pad);
        return rc;
    }

    pad->transfer_buffer_size = endpoint->wMaxPacketSize;
    pad->transfer_buffer = usb_alloc_coherent(pad->usb_device,
                                              pad->transfer_buffer_size,
                                              GFP_KERNEL, &pad->dma);
    if (!pad->transfer_buffer) {
        LOG_ERR_INTF_0("\ttransfer buffer allocation FAILURE\n");
        input_free_device(pad->input_device);
        kfree(pad);
        return rc;
    }

    usb_make_path(pad->usb_device, pad->phys, sizeof(pad->phys));
    strlcat(pad->phys, "/input0", sizeof(pad->phys));

    pad->input_device->name = "Huion H640P Drawpad Interface 0";
    pad->input_device->phys = pad->phys;
    usb_to_input_id(pad->usb_device, &pad->input_device->id);
    pad->input_device->dev.parent = &interface->dev;

    input_set_drvdata(pad->input_device, pad);

    pad->input_device->open = open_interface_0;
    pad->input_device->close = close_interface_0;

    for (int i = 0; i < (sizeof(input_event_types) / sizeof(int)); i++) {
        set_bit(input_event_types[i], pad->input_device->evbit);
    }

    pad->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!pad->urb) {
        LOG_ERR_INTF_0("\tusb_alloc_urb FAILURE\n");
        input_free_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
        return rc;
    }

    pad->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    int urb_pipe = usb_rcvintpipe(pad->usb_device, endpoint->bEndpointAddress);
    LOG_INFO_INTF_0("endpoint address: 0x%x\n", endpoint->bEndpointAddress);

    usb_fill_int_urb(pad->urb, pad->usb_device, urb_pipe,
                     pad->transfer_buffer, pad->transfer_buffer_size,
                     pad_irq, pad,
                     endpoint->bInterval);
    pad->urb->transfer_dma = pad->dma;
    
    rc = input_register_device(pad->input_device);
    if (rc == 0) {
        usb_set_intfdata(interface, pad);
        pad_init_pen_status(pad);

        LOG_INFO_INTF_0("interface->minor %d\n", interface->minor);
    
    } else {
        LOG_ERR_INTF_0("\tinput_register_device FAILURE\n");
        usb_free_urb(pad->urb);
        input_free_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
    }

    return rc;
}

static void disconnect_interface_0(struct usb_interface *interface) {
    
    struct drawpad *pad = usb_get_intfdata(interface);
    if (pad) {
        usb_kill_urb(pad->urb);
        usb_free_urb(pad->urb);
        input_unregister_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
    }
}


#endif // __DRAWPAD_DRIVER__