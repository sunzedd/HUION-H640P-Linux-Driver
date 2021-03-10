#ifndef __HUION_DRAWPAD_PAD__
#define __HUION_DRAWPAD_PAD__

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <linux/interrupt.h>

#include "fetch_dev_info.h"

#define MAX_PEN_PRESSURE            8191
#define MAX_PAD_RESOLUTION_VALUE    32767

#define MAX_SCREEN_X    1920
#define MAX_SCREEN_Y    1080

#define X_FACTOR    MAX_SCREEN_X / MAX_PAD_RESOLUTION_VALUE + 1
#define Y_FACTOR    MAX_SCREEN_Y / MAX_PAD_RESOLUTION_VALUE + 1


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

static
void pad_init_pen_status(struct pad *pad) {
    pad->pen_touchdown = 0;
    pad->pen_above_pad = 0;
}

static const int input_event_types[] = {
    EV_ABS,
    EV_KEY,
};

static const int abs_events[] = {
    ABS_X,
    ABS_Y,
    ABS_PRESSURE,
};

static const int button_events[] = {
    BTN_TOOL_PEN,
    BTN_STYLUS,       
    BTN_STYLUS2, 
    BTN_TOUCH,
};

static const int drawpad_properties[] = {
    INPUT_PROP_DIRECT,
    INPUT_PROP_POINTER,
};

static void tasklet_handler(unsigned long tasklet_data) {
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
        LOG_ERR_PAD("Invalid packet recieved. Header = %x\n", header);
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

    LOG_INFO_PAD("%x %x, %hu, %hu\n", header, pen_status, x, y);
    LOG_INFO_PAD("%x %x %x %x %x\n", header, pen_status, x, y, pressure);

    input_sync(pad->input_device);
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
            LOG_ERR_PAD("\tusb_submit_urb failed\n");
        }

    } else {
        LOG_ERR_PAD("\tError urb status recieved: ");
        switch (urb->status) {
            case -ENOENT: LOG_ERR_PAD("\t\tENOENT (killed by usb_kill_urb)\n"); break;
            case -ECONNRESET: LOG_ERR_PAD("\t\tECONNRESET\n"); break;
            case -EINPROGRESS: LOG_ERR_PAD("\t\tEINPROGRESS\n"); break;
            default: LOG_ERR_PAD("\t\tanother error: %d\n", urb->status); break;
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
        LOG_ERR_PAD("\tstruct pad allocation FAILURE\n");
        return rc;
    }

    pad->usb_device = interface_to_usbdev(interface);
    pad->input_device = input_allocate_device();
    if (!pad->input_device) {
        LOG_ERR_PAD("\tinput_allocate_device FAILURE\n");
        kfree(pad);
        return rc;
    }

    pad->transfer_buffer_size = endpoint->wMaxPacketSize;
    pad->transfer_buffer = usb_alloc_coherent(pad->usb_device,
                                              pad->transfer_buffer_size,
                                              GFP_KERNEL, &pad->dma);
    if (!pad->transfer_buffer) {
        LOG_ERR_PAD("\ttransfer buffer allocation FAILURE\n");
        input_free_device(pad->input_device);
        kfree(pad);
        return rc;
    }

    usb_make_path(pad->usb_device, pad->phys, sizeof(pad->phys));
    strlcat(pad->phys, "/input0", sizeof(pad->phys));

    pad->input_device->name = "Huion H640P Pad";
    pad->input_device->phys = pad->phys;
    usb_to_input_id(pad->usb_device, &pad->input_device->id);
    pad->input_device->dev.parent = &interface->dev;

    input_set_drvdata(pad->input_device, pad);

    pad->input_device->open = pad_open;
    pad->input_device->close = pad_close;


    for (int i = 0; i < (sizeof(input_event_types) / sizeof(int)); i++) {
        set_bit(input_event_types[i], pad->input_device->evbit);
    }

    for (int i = 0; i < (sizeof(abs_events) / sizeof(int)); i++) {
        set_bit(abs_events[i], pad->input_device->absbit);
    }
    
    for (int i = 0; i < (sizeof(button_events) / sizeof(int)); i++) {
        set_bit(button_events[i], pad->input_device->keybit);
    }

    for (int i = 0; i < (sizeof(drawpad_properties) / sizeof(int)); i++) {
        set_bit(drawpad_properties[i], pad->input_device->propbit);
    }

    input_set_abs_params(pad->input_device, ABS_X, 0, MAX_SCREEN_X, 0, 0);
    input_abs_set_res(pad->input_device, ABS_X, MAX_PAD_RESOLUTION_VALUE);
    input_set_abs_params(pad->input_device, ABS_Y, 0, MAX_SCREEN_Y, 0, 0);
    input_abs_set_res(pad->input_device, ABS_Y, MAX_PAD_RESOLUTION_VALUE);
    input_set_abs_params(pad->input_device, ABS_PRESSURE, 0, MAX_PEN_PRESSURE, 0, 0);

    pad->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!pad->urb) {
        LOG_ERR_PAD("\tusb_alloc_urb FAILURE\n");
        input_free_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
        return rc;
    }

    pad->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    int urb_pipe = usb_rcvintpipe(pad->usb_device, endpoint->bEndpointAddress);


    usb_fill_int_urb(pad->urb, pad->usb_device, urb_pipe,
                     pad->transfer_buffer, pad->transfer_buffer_size,
                     pad_irq, pad,
                     endpoint->bInterval);
    pad->urb->transfer_dma = pad->dma;
    
    rc = input_register_device(pad->input_device);
    if (rc == 0) {
        
        usb_set_intfdata(interface, pad);
        pad_init_pen_status(pad);

    } else {
        LOG_ERR_PAD("\tinput_register_device FAILURE\n");
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

#endif // __HUION_DRAWPAD_PAD__