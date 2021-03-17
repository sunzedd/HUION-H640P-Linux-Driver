#ifndef __DRAWPAD_DRIVER__
#define __DRAWPAD_DRIVER__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <linux/interrupt.h>

#include "log_utils.h"
#include "drawpad_properties.h"

#define DRIVER_NAME     "Huion H640P Interface 1 Driver"
#define DRIVER_AUTHOR   "Rostislav V."


static int probe(struct usb_interface *interface, 
                 const struct usb_device_id *dev_id);
static int probe_interface_1(struct usb_interface *interface,
                             const struct usb_device_id *dev_id);
static int open_interface_1(struct input_dev* input_device);

static void disconnect(struct usb_interface* interface);
static void disconnect_interface_1(struct usb_interface* interface);
static void close_interface_1(struct input_dev* input_device);


static struct usb_device_id devices_table[] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, 
    {}
};

MODULE_DEVICE_TABLE(usb, devices_table);

static struct usb_driver drawpad_driver = {
    .name = DRIVER_NAME,
    .id_table = devices_table,
    .probe = probe,
    .disconnect = disconnect,
};

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


struct drawpad {
    char    phys[32];
    struct usb_device   *usb_device;
    struct input_dev    *input_device;
    struct urb      *urb;
    unsigned char   *transfer_buffer;
    unsigned int    transfer_buffer_size;
    dma_addr_t      dma;
};

static struct drawpad *drawpad;


static void tasklet_handler(unsigned long tasklet_data) {
    uint8_t header;
    uint8_t pen_status;
    uint16_t x; 
    uint16_t y; 
    uint16_t pressure; 

    unsigned char *data = drawpad->transfer_buffer;

    header = data[0];
    pen_status = data[1];
    memcpy(&x, &data[2], 2);
    memcpy(&y, &data[4], 2);
    memcpy(&pressure, &data[6], 2);

    if (header != 0xa) {
        LOG_ERR("Invalid packet recieved. Header = %x\n", header);
        return;
    }

    input_report_key(drawpad->input_device, BTN_TOOL_PEN, pen_status & 0xc0);
    input_report_key(drawpad->input_device, BTN_TOUCH, pen_status & 0x1);
    input_report_key(drawpad->input_device, BTN_STYLUS, pen_status & 0x2);
    input_report_key(drawpad->input_device, BTN_STYLUS, pen_status & 0x4);

    input_report_abs(drawpad->input_device, ABS_X, x);
    input_report_abs(drawpad->input_device, ABS_Y, y);
    input_report_abs(drawpad->input_device, ABS_PRESSURE, pressure);

    input_sync(drawpad->input_device);
}

DECLARE_TASKLET(drawpad_tasklet, tasklet_handler, 0);


static void drawpad_irq(struct urb *urb) {
    struct drawpad *drawpad = urb->context;
    if (urb->status == 0) {
        tasklet_schedule(&drawpad_tasklet);

        int rc = usb_submit_urb(drawpad->urb, GFP_ATOMIC);
        if (rc) {
            LOG_ERR("\tfailed to submit urb\n");
        }
    } else {
        LOG_WARN("warning: urb status recieved: ");
        switch (urb->status) {
            case -ENOENT:
                LOG_ERR("\tENOENT (killed by usb_kill_urb)\n");
                break;
            default:
                LOG_ERR("\tanother error: %d\n", urb->status);
                break;
        }
    }
}


static int probe(struct usb_interface *interface,
                 const struct usb_device_id *dev_id) {
                     
    int rc = -ENODEV;

    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    LOG_INFO("probe device (%04x:%04X) Interface: %d\n",
                    dev_id->idVendor, dev_id->idProduct, interface_number);

    if (interface_number == 1) {
        rc = probe_interface_1(interface, dev_id);
    }
   
    return rc;
}

static int probe_interface_1(struct usb_interface *interface,
                             const struct usb_device_id *dev_id) {

    int rc = -ENOMEM;

    struct usb_endpoint_descriptor *endpoint = 
        &interface->cur_altsetting->endpoint[0].desc;

    drawpad = kzalloc(sizeof(struct drawpad), GFP_KERNEL);
    if (!drawpad) {
        LOG_ERR("\tstruct drawpad allocation FAILURE\n");
        return rc;
    }

    drawpad->usb_device = interface_to_usbdev(interface);
    drawpad->input_device = input_allocate_device();
    if (!drawpad->input_device) {
        LOG_ERR("\tinput_allocate_device FAILURE\n");
        kfree(drawpad);
        return rc;
    }

    drawpad->transfer_buffer_size = endpoint->wMaxPacketSize;
    drawpad->transfer_buffer = usb_alloc_coherent(drawpad->usb_device,
                                              drawpad->transfer_buffer_size,
                                              GFP_KERNEL, &drawpad->dma);
    if (!drawpad->transfer_buffer) {
        LOG_ERR("\ttransfer buffer allocation FAILURE\n");
        input_free_device(drawpad->input_device);
        kfree(drawpad);
        return rc;
    }

    usb_make_path(drawpad->usb_device, drawpad->phys, sizeof(drawpad->phys));
    strlcat(drawpad->phys, "/input1", sizeof(drawpad->phys));

    drawpad->input_device->name = "Huion H640P Drawpad Interface 1";
    drawpad->input_device->phys = drawpad->phys;
    usb_to_input_id(drawpad->usb_device, &drawpad->input_device->id);
    drawpad->input_device->dev.parent = &interface->dev;

    input_set_drvdata(drawpad->input_device, drawpad);

    drawpad->input_device->open = open_interface_1;
    drawpad->input_device->close = close_interface_1;

    for (int i = 0; i < (sizeof(input_event_types) / sizeof(int)); i++) {
        set_bit(input_event_types[i], drawpad->input_device->evbit);
    }

    for (int i = 0; i < (sizeof(abs_events) / sizeof(int)); i++) {
        set_bit(abs_events[i], drawpad->input_device->absbit);
    }
    
    for (int i = 0; i < (sizeof(button_events) / sizeof(int)); i++) {
        set_bit(button_events[i], drawpad->input_device->keybit);
    }

    for (int i = 0; i < (sizeof(drawpad_properties) / sizeof(int)); i++) {
        set_bit(drawpad_properties[i], drawpad->input_device->propbit);
    }

    input_set_abs_params(drawpad->input_device, ABS_X, 0, MAX_ABS_X, 0, 0);
    input_abs_set_res(drawpad->input_device, ABS_X, MAX_PAD_RESOLUTION_VALUE);
    input_set_abs_params(drawpad->input_device, ABS_Y, 0, MAX_ABS_Y, 0, 0);
    input_abs_set_res(drawpad->input_device, ABS_Y, MAX_PAD_RESOLUTION_VALUE);
    input_set_abs_params(drawpad->input_device, ABS_PRESSURE,
                         0, MAX_PEN_PRESSURE, 0, 0);

    drawpad->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!drawpad->urb) {
        LOG_ERR("\tusb_alloc_urb FAILURE\n");
        input_free_device(drawpad->input_device);
        usb_free_coherent(drawpad->usb_device, drawpad->transfer_buffer_size,
                          drawpad->transfer_buffer, drawpad->dma);
        kfree(drawpad);
        return rc;
    }

    drawpad->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    int pipe = usb_rcvintpipe(drawpad->usb_device, endpoint->bEndpointAddress);
    LOG_INFO("endpoint address: 0x%x\n", endpoint->bEndpointAddress);

    usb_fill_int_urb(drawpad->urb, drawpad->usb_device, pipe,
                     drawpad->transfer_buffer, drawpad->transfer_buffer_size,
                     drawpad_irq, drawpad,
                     endpoint->bInterval);
    drawpad->urb->transfer_dma = drawpad->dma;
    
    rc = input_register_device(drawpad->input_device);
    if (rc == 0) {
        usb_set_intfdata(interface, drawpad);
    } else {
        LOG_ERR("\tinput_register_device FAILURE\n");
        usb_free_urb(drawpad->urb);
        input_free_device(drawpad->input_device);
        usb_free_coherent(drawpad->usb_device, drawpad->transfer_buffer_size,
                          drawpad->transfer_buffer, drawpad->dma);
        kfree(drawpad);
    }

    return rc;
}

static void disconnect(struct usb_interface* interface) {    
    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    int interface_number = interface_desc->desc.bInterfaceNumber;
    
    if (interface_number == 1) {
        disconnect_interface_1(interface);
    }
}

static void disconnect_interface_1(struct usb_interface *interface) {
    LOG_INFO("disconnect device\n");
    
    struct drawpad *drawpad = usb_get_intfdata(interface);
    if (drawpad) {
        usb_kill_urb(drawpad->urb);
        usb_free_urb(drawpad->urb);
        input_unregister_device(drawpad->input_device);
        usb_free_coherent(drawpad->usb_device, drawpad->transfer_buffer_size,
                          drawpad->transfer_buffer, drawpad->dma);
        kfree(drawpad);
    }
}


static int open_interface_1(struct input_dev* input_device) {
    struct drawpad *drawpad = input_get_drvdata(input_device);
    if (usb_submit_urb(drawpad->urb, GFP_KERNEL)) {
        return -EIO;
    }

    return 0;
}

static void close_interface_1(struct input_dev* input_device) {
    struct drawpad *drawpad = input_get_drvdata(input_device);
    usb_kill_urb(drawpad->urb);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);

module_init(drawpad_driver_init);
module_exit(drawpad_driver_exit);


#endif // __DRAWPAD_DRIVER__