#ifndef __HUION_DRAWPAD_PAD__
#define __HUION_DRAWPAD_PAD__

#include <linux/init.h>
#include "fetch_dev_info.h"

#define PAD_ENDPOINT_ADDRESS 0x82

static struct usb_class_driver pad_class_driver;


static int pad_open(struct inode *i, struct file *f) { return 0; }
static int pad_close(struct inode *i, struct file *f) { return 0; }

static
ssize_t pad_read(struct file *f, char __user *buf, size_t cnt, loff_t *off) {
    // Just log for now
    LOG_INFO("call pad_read\n");
    return 1;
}

static 
ssize_t pad_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off) {
    // Just log for now
    LOG_INFO("call pad_write\n");
    return 1;
}

static struct file_operations pad_fops = {
    .open = pad_open,
    .release = pad_close,
    .read = pad_read,
    .write = pad_write
};

static
int pad_probe(struct usb_interface *interface,
              const struct usb_device_id *dev_id) {

    LOG_INFO("call pad_probe\n");    
    print_usb_interface_description(interface);

    pad_class_driver.name = "usb/huion_pad";
    pad_class_driver.fops = &pad_fops;

    int rc = usb_register_dev(interface, &pad_class_driver);    // Этот вызов создает файл устройства (интерфеса) в dev
    if (rc < 0) {
        LOG_ERR("usb_register_dev FAILURE\n");
    } else { 
        LOG_INFO("Registered Pad with a MINOR: %d\n", interface->minor);
    }

    return rc;
}

static
void pad_disconnect(struct usb_interface *interface) {

    LOG_INFO("call pad_disconnect\n");
    usb_deregister_dev(interface, &pad_class_driver);
}

#endif // __HUION_DRAWPAD_PAD__