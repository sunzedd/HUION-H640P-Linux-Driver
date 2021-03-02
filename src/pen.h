#ifndef __HUION_DRAWPAD_PEN__
#define __HUION_DRAWPAD_PEN__

#include <linux/init.h>
#include "fetch_dev_info.h"

#define PEN_ENDPOINT_ADDRESS 0x81

static struct usb_class_driver pen_class_driver;


static int pen_open(struct inode *i, struct file *f) { return 0; }
static int pen_close(struct inode *i, struct file *f) { return 0; }

static
ssize_t pen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off) {
    // Just log for now
    LOG_INFO("call pen_read\n");
    return 1;
}

static 
ssize_t pen_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off) {
    // Just log for now
    LOG_INFO("call pen_write\n");
    return 1;
}

static struct file_operations pen_fops = {
    .open = pen_open,
    .release = pen_close,
    .read = pen_read,
    .write = pen_write
};


static
int pen_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("call pen_probe\n");
    print_usb_interface_description(interface);

    pen_class_driver.name = "usb/huion_pen";
    pen_class_driver.fops = &pen_fops;

    int rc = usb_register_dev(interface, &pen_class_driver);    // Этот вызов создает файл устройства (интерфеса) в dev
    if (rc < 0) {
        LOG_ERR("usb_register_dev FAILURE\n");
    } else { 
        LOG_INFO("Registered Pen with a MINOR: %d\n", interface->minor);
    }

    return rc;
}

static
void pen_disconnect(struct usb_interface *interface) {

    LOG_INFO("call pen_disconnect\n");
    usb_deregister_dev(interface, &pen_class_driver);
}

#endif // __HUION_DRAWPAD_PEN__
