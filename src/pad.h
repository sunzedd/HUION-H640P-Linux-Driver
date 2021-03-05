#ifndef __HUION_DRAWPAD_PAD__
#define __HUION_DRAWPAD_PAD__

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb/input.h>

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
    unsigned char   *transfer_buffer;   // буфер для чтения данных из устройства.
    unsigned int    transfer_buffer_size;
    dma_addr_t      dma;
};

static
void pad_parse_transfer_buffer(struct pad *pad) {
    uint8_t first_byte;
    uint8_t pen_status;
    uint16_t x; 
    uint16_t y; 
    uint16_t pressure; 

    unsigned char *data = pad->transfer_buffer;

    first_byte = data[0];
    pen_status = data[1];
    memcpy(&x, &data[2], 2);
    memcpy(&y, &data[4], 2);
    memcpy(&pressure, &data[6], 2);
    
    LOG_INFO_PAD("recv packet: %02x %02x (x: %02x = %hu) (y: %02x = %hu) (pressure: %hu)\n",
                  first_byte, pen_status, x, x, y, y, pressure);
    
    input_report_abs(pad->input_device, ABS_X, x * X_FACTOR);
    input_report_abs(pad->input_device, ABS_Y, y * Y_FACTOR);
    input_report_abs(pad->input_device, ABS_PRESSURE, pressure);
    input_sync(pad->input_device);

    LOG_INFO_PAD("reported x,y: %d, %d\n", x * X_FACTOR, y * Y_FACTOR);
}


static
void pad_irq(struct urb *urb) {
    int rc = 0;

    struct pad *pad = urb->context;

    if (urb->status == 0) {
        pad_parse_transfer_buffer(pad);

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
    LOG_INFO_PAD("pad opened\n");
    
    struct pad *pad = input_get_drvdata(input_device);

    if (usb_submit_urb(pad->urb, GFP_KERNEL)) {
        return -EIO;
    }

    return 0;
}

static
void pad_close(struct input_dev* input_device) {
    LOG_INFO_PAD("pad closed\n");

    struct pad *pad = input_get_drvdata(input_device);
    usb_kill_urb(pad->urb);
}

static
int pad_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO_PAD("\tcall pad_probe\n");
    
    int rc = -ENOMEM;

    struct usb_endpoint_descriptor *endpoint = 
        &interface->cur_altsetting->endpoint[0].desc;

    struct pad *pad = kzalloc(sizeof(struct pad), GFP_KERNEL);
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

    LOG_INFO_PAD("\tendpoint->wMaxPacketSize %d\n", endpoint->wMaxPacketSize);

    usb_make_path(pad->usb_device, pad->phys, sizeof(pad->phys));
    strlcat(pad->phys, "/input0", sizeof(pad->phys));

    pad->input_device->name = "Huion H640P Pad";
    pad->input_device->phys = pad->phys;
    usb_to_input_id(pad->usb_device, &pad->input_device->id);
    pad->input_device->dev.parent = &interface->dev;

    input_set_drvdata(pad->input_device, pad);

    pad->input_device->open = pad_open;
    pad->input_device->close = pad_close;

//----------------------------------------------------------------------------
    // Типы событий
    __set_bit(EV_ABS, pad->input_device->evbit);
    __set_bit(EV_KEY, pad->input_device->evbit);

    // События абсолютного ввода
    __set_bit(ABS_X, pad->input_device->absbit);
    __set_bit(ABS_Y, pad->input_device->absbit);
    __set_bit(ABS_PRESSURE, pad->input_device->absbit);

    // События кнопок
    __set_bit(BTN_TOUCH, pad->input_device->keybit);
    __set_bit(BTN_TOOL_PEN, pad->input_device->keybit);
    __set_bit(BTN_TOOL_RUBBER, pad->input_device->keybit);
    __set_bit(BTN_STYLUS2, pad->input_device->keybit);
    __set_bit(BTN_STYLUS, pad->input_device->keybit);
    __set_bit(BTN_LEFT, pad->input_device->keybit);

    input_set_abs_params(pad->input_device, ABS_X, 0, MAX_SCREEN_X, 0, 0);
    input_abs_set_res(pad->input_device, ABS_X, MAX_PAD_RESOLUTION_VALUE);

    input_set_abs_params(pad->input_device, ABS_Y, 0, MAX_SCREEN_Y, 0, 0);
    input_abs_set_res(pad->input_device, ABS_Y, MAX_PAD_RESOLUTION_VALUE);

    input_set_abs_params(pad->input_device, ABS_PRESSURE, 0, MAX_PEN_PRESSURE, 0, 0);

    //__set_bit(INPUT_PROP_DIRECT, pad->input_device->propbit);

    pad->urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!pad->urb) {
        LOG_ERR_PAD("\tusb_alloc_urb FAILURE\n");
        input_free_device(pad->input_device);
        usb_free_coherent(pad->usb_device, pad->transfer_buffer_size,
                          pad->transfer_buffer, pad->dma);
        kfree(pad);
        return rc;
    }

    // Флаг, указывающий требование использовать DMA буфер вместо transfer_buffer
    pad->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    
    int urb_pipe = usb_rcvintpipe(pad->usb_device, endpoint->bEndpointAddress);

    LOG_INFO_PAD("\tendpoint->bEndpointAddress = %x\n", endpoint->bEndpointAddress);
    LOG_INFO_PAD("\turb rcvintpipe = %d\n", urb_pipe);


    usb_fill_int_urb(pad->urb, pad->usb_device, urb_pipe,
                     pad->transfer_buffer, pad->transfer_buffer_size,
                     pad_irq, pad,
                     endpoint->bInterval);
    pad->urb->transfer_dma = pad->dma;
    
    rc = input_register_device(pad->input_device);
    if (rc == 0) {
        usb_set_intfdata(interface, pad);
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
    
    LOG_INFO_PAD("\tcall pad_disconnect\n");

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