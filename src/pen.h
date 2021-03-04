#ifndef __HUION_DRAWPAD_PEN__
#define __HUION_DRAWPAD_PEN__

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>

#include "fetch_dev_info.h"

#define PEN_USB_PACKET_SIZE     0x0040  // 64 bytes

typedef struct pen {
    char    phys[32];

    struct usb_device   *usb_device;
    struct input_dev    *input_device;

    struct urb      *urb;
    unsigned char   *transfer_buffer;   // буфер для чтения данных из устройства.  
    unsigned int    transfer_buffer_size;
} pen_t;


static pen_t pen_object;


static
void pen_irq(struct urb *urb) {
    LOG_INFO("\tpen_irq callback called\n");
}

static
int pen_open(struct input_dev* input_device) {
    LOG_INFO("\tcall pen_open()\n");
    return 1;
}

static
void pen_close(struct input_dev* input_device) {
    LOG_INFO("\tcall pen_close()\n");
}

static
int pen_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("\tcall pen_probe\n");
    
    int rc = 0;

    pen_object.usb_device = interface_to_usbdev(interface);
    pen_object.input_device = input_allocate_device();
    if (pen_object.input_device == NULL) {
        LOG_ERR("\tinput_allocate_device() FAILURE\n");
        return -1;
    }

    usb_make_path(pen_object.usb_device,
                  pen_object.phys, sizeof(pen_object.phys));
    strlcat(pen_object.phys, "/input0", sizeof(pen_object.phys));

    pen_object.input_device->name = "Huion H640P Pen";
    pen_object.input_device->phys = pen_object.phys;
    usb_to_input_id(pen_object.usb_device, &pen_object.input_device->id);
    pen_object.input_device->dev.parent = &interface->dev;
    LOG_INFO("\tpen usb device path: %s", pen_object.input_device->phys);

    pen_object.input_device->open = pen_open;
    pen_object.input_device->close = pen_close;

    rc = input_register_device(pen_object.input_device);
    if (rc != 0) {
        LOG_ERR("\tinput_register_revice() FAILURE\n");
        return -1;
    }

    if (rc == 0) { 
        struct usb_host_interface *intf_desc = interface->cur_altsetting;
        struct usb_endpoint_descriptor *endpoint_desc = &(intf_desc->endpoint[0].desc); // В устройстве по одному endpoint-у на каждый интерфейс.
        
        pen_object.transfer_buffer_size = endpoint_desc->wMaxPacketSize;
        pen_object.transfer_buffer = kzalloc(pen_object.transfer_buffer_size, GFP_KERNEL);
        if (pen_object.transfer_buffer == NULL) {
            LOG_ERR("\tpen_object.transfer_buffer allocation FAILURE\n");
            return -1;
        }

        pen_object.urb = usb_alloc_urb(0, GFP_KERNEL);
        if (pen_object.urb == NULL) { 
            LOG_ERR("\tpen_object.urb = usb_alloc_urb() FAILURE\n");
            return -1;
        }

        usb_fill_int_urb(pen_object.urb, pen_object.usb_device,
                         usb_rcvintpipe(pen_object.usb_device, endpoint_desc->bEndpointAddress),
                         pen_object.transfer_buffer, pen_object.transfer_buffer_size,
                         pen_irq, &pen_object,
                         endpoint_desc->bInterval);

        LOG_INFO("\tusb_fill_int_urb settings:\n");
        LOG_INFO("\t\tbEndpointAddress %x, wMaxPacketSize %d, bInterval %d",
                      endpoint_desc->bEndpointAddress,
                      pen_object.transfer_buffer_size,
                      endpoint_desc->bInterval);

        rc = usb_submit_urb(pen_object.urb, GFP_ATOMIC);
        if (rc != 0) {
            LOG_ERR("\tusb_submit_urb(pen, ...) FAILURE\n");
            return rc; 
        }

        LOG_INFO("\tpen submited URB\n");
    }

    return rc;
}

static
void pen_disconnect(struct usb_interface *interface) {

    LOG_INFO("\tcall pen_disconnect\n");

    input_unregister_device(pen_object.input_device);
    input_free_device(pen_object.input_device);
    usb_kill_urb(pen_object.urb);
}

#endif // __HUION_DRAWPAD_PEN__
