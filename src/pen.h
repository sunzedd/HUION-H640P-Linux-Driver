#ifndef __HUION_DRAWPAD_PEN__
#define __HUION_DRAWPAD_PEN__

#include <linux/init.h>
#include <linux/slab.h>
#include "fetch_dev_info.h"

#define PEN_ENDPOINT_ADDRESS    0x81    // IN, TransferType = Interrupt, bInterval = 2 ms (Интервал между прерываниями от устройства)
#define INTERRUPT_INTERVAL_MS   2
#define PEN_USB_PACKET_SIZE     0x0040  // 64 bytes

typedef
struct pen { 
    struct usb_device   *usb_device;
    unsigned char       *transfer_buffer;   // буфер для чтения данных из устройства.  
    unsigned int        buffer_size;
    struct urb          *urb;
} pen_t;


static struct usb_class_driver pen_class_driver;
static pen_t pen_object;


static int pen_open(struct inode *i, struct file *f) {
    LOG_INFO("\tcall pen_open\n");
    return 0; 
}

static int pen_close(struct inode *i, struct file *f) {
    LOG_INFO("\tcall pen_close\n");
    return 0;
}

static
ssize_t pen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off) {
    // Just log for now
    LOG_INFO("\tcall pen_read\n");
    LOG_INFO("\t\ttransfer_buffer: %s\n", pen_object.transfer_buffer);
    return 1;
}

static 
ssize_t pen_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off) {
    // Just log for now
    LOG_INFO("\tcall pen_write\n");
    return 1;
}

static struct file_operations pen_fops = {
    .open = pen_open,
    .release = pen_close,
    .read = pen_read,
    .write = pen_write
};

static
void pen_irq(struct urb *urb) {
    LOG_INFO("\tpen_irq callback called\n");
}


static
int pen_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("\tcall pen_probe\n");
    print_usb_interface_description(interface);

    pen_object.usb_device = interface_to_usbdev(interface);

    pen_class_driver.name = "usb/huion_pen";
    pen_class_driver.fops = &pen_fops;

    int rc = usb_register_dev(interface, &pen_class_driver);    // Этот вызов создает файл устройства (интерфеса) в dev
    if (rc < 0) {
        LOG_ERR("\tusb_register_dev FAILURE\n");
    } else { 
        LOG_INFO("\tregistered Pen with a MINOR: %d\n", interface->minor);
    }

    if (rc == 0) { 
        struct usb_host_interface *intf_desc = interface->cur_altsetting;
        struct usb_endpoint_descriptor *endpoint_desc = &(intf_desc->endpoint[0].desc); // В устройстве по одному endpoint-у на каждый интерфейс.
        
        pen_object.buffer_size = endpoint_desc->wMaxPacketSize;
        pen_object.transfer_buffer = kzalloc(pen_object.buffer_size, GFP_KERNEL);
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
                         pen_object.transfer_buffer, pen_object.buffer_size,
                         pen_irq, &pen_object,
                         endpoint_desc->bInterval);

        LOG_INFO("\tusb_fill_int_urb settings:\n");
        LOG_INFO("\t\tbEndpointAddress %x, wMaxPacketSize %d, bInterval %d",
                      endpoint_desc->bEndpointAddress,
                      pen_object.buffer_size,
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

    usb_kill_urb(pen_object.urb);
    usb_free_urb(pen_object.urb);
    usb_deregister_dev(interface, &pen_class_driver);
}

#endif // __HUION_DRAWPAD_PEN__
