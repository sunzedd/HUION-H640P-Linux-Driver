#ifndef __HUION_DRAWPAD_PEN__
#define __HUION_DRAWPAD_PEN__

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>

#include "fetch_dev_info.h"


struct pen {
    char    phys[32];

    struct usb_device   *usb_device;
    struct input_dev    *input_device;

    struct urb      *urb;

    unsigned char   *transfer_buffer;   // буфер для чтения данных из устройства.  
    unsigned int    transfer_buffer_size;
    dma_addr_t      dma;
};


static
void pen_irq(struct urb *urb) {
    LOG_INFO("\tpen_irq callback called\n");
}

static
int pen_open(struct input_dev* input_device) {
    LOG_INFO("\tcall pen_open()\n");
    
    return 0;
}

static
void pen_close(struct input_dev* input_device) {
    LOG_INFO("\tcall pen_close()\n");
}

static
int pen_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("\tcall pen_probe\n");
    
    int rc = -ENOMEM;

    struct usb_endpoint_descriptor *endpoint = 
        &interface->cur_altsetting->endpoint[0].desc;

    struct pen *pen = kzalloc(sizeof(struct pen), GFP_KERNEL);
    if (!pen) {
        LOG_ERR("\tstruct pen allocation FAILURE\n");
        return rc;
    }

    pen->usb_device = interface_to_usbdev(interface);
    pen->input_device = input_allocate_device();
    if (!pen->input_device) {
        LOG_ERR("\tinput_allocate_device FAILURE\n");
        kfree(pen);
        return rc;
    }

    pen->transfer_buffer_size = endpoint->wMaxPacketSize;
    pen->transfer_buffer = usb_alloc_coherent(pen->usb_device,
                                              pen->transfer_buffer_size,
                                              GFP_KERNEL, &pen->dma);
    if (!pen->transfer_buffer) {
        LOG_ERR("\ttransfer buffer allocation FAILURE\n");
        input_free_device(pen->input_device);
        kfree(pen);
        return rc;
    }

    pen->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!pen->urb) {
        LOG_ERR("\tusb_alloc_urb FAILURE\n");
        input_free_device(pen->input_device);
        usb_free_coherent(pen->usb_device, pen->transfer_buffer_size,
                          pen->transfer_buffer, pen->dma);
        kfree(pen);
        return rc;
    }

    usb_make_path(pen->usb_device, pen->phys, sizeof(pen->phys));
    strlcat(pen->phys, "/input0", sizeof(pen->phys));

    pen->input_device->name = "Huion H640P Pen";
    pen->input_device->phys = pen->phys;
    usb_to_input_id(pen->usb_device, &pen->input_device->id);
    pen->input_device->dev.parent = &interface->dev;

    input_set_drvdata(pen->input_device, pen);

    pen->input_device->open = pen_open;
    pen->input_device->close = pen_close;

    int urb_pipe = usb_rcvintpipe(pen->usb_device, endpoint->bEndpointAddress);
    usb_fill_int_urb(pen->urb, pen->usb_device, urb_pipe,
                     pen->transfer_buffer, pen->transfer_buffer_size,
                     pen_irq, pen,
                     endpoint->bInterval);
    pen->urb->transfer_dma = pen->dma;
    
    rc = input_register_device(pen->input_device);
    if (rc == 0) {
        usb_set_intfdata(interface, pen);
    } else {
        LOG_ERR("\tinput_register_device FAILURE\n");
        usb_free_urb(pen->urb);
        input_free_device(pen->input_device);
        usb_free_coherent(pen->usb_device, pen->transfer_buffer_size,
                          pen->transfer_buffer, pen->dma);
        kfree(pen);
    }

    if (rc == 0) {
        LOG_INFO("\tRegistered Pen input device\n");
        LOG_INFO("\tPen phys: %s\n", pen->phys);
    }

    return rc;
}

static
void pen_disconnect(struct usb_interface *interface) {
    
    LOG_INFO("\tcall pen_disconnect\n");

    struct pen *pen = usb_get_intfdata(interface);
    if (pen) {
        usb_kill_urb(pen->urb);
        usb_free_urb(pen->urb);
        input_unregister_device(pen->input_device);
        usb_free_coherent(pen->usb_device, pen->transfer_buffer_size,
                          pen->transfer_buffer, pen->dma);
        kfree(pen);
    }
}

#endif // __HUION_DRAWPAD_PEN__
