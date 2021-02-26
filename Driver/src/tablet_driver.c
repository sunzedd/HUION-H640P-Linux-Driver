#ifndef HUION_DRIVER_H
#define HUION_DRIVER_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define DRIVER_DESC     "Linux LKM Driver for HUION H640P Pen Tablet"
#define DRIVER_AUTHOR   "Rostislav V."
#define DRIVER_NAME     "HUION H640P Custom Driver"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

#define USB_VENDOR_ID       0x256c  /* Huion Animation Technology Co. */
#define USB_PRODUCT_ID      0x006d  /* Pen Tablet HUION H640P */

/*
    Указание использовать данный драйвер только для конкретного
    планшета с идентификаторами производителя и продукта 
    USB_VENDOR_ID, USB_PRODUCT_ID соответственно.
*/
static struct usb_device_id tablet_table[] = 
{
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    { USB_DEVICE(USB_VENDOR_ID, 0x006e) },
    { }
};

MODULE_DEVICE_TABLE(usb, tablet_table);


static int tablet_probe(struct usb_interface *interface,
                        const struct usb_device_id *dev_id)
{
    printk(KERN_INFO "Pen Tablet (%04X:%04X) plugged in\n",
           dev_id->idVendor, dev_id->idProduct);
    return 0;
}

static void tablet_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Pen Tablet disconnected\n");
}


static struct usb_driver tablet_driver =
{
    .name = DRIVER_NAME,
    .id_table = tablet_table,
    .probe = tablet_probe,
    .disconnect = tablet_disconnect,
};



static int __init tablet_init(void)
{
    int rc = usb_register(&tablet_driver);
    if (rc != 0)
    {
        printk(KERN_ERR "%s Loading Failure\n", DRIVER_NAME);
    }
    else
    {
        printk(KERN_INFO "%s Loaded successfuly\n", DRIVER_NAME);
    }

    return rc;
}

static void __exit tablet_exit(void)
{
    usb_deregister(&tablet_driver);
    printk(KERN_INFO "%s Unloaded\n", DRIVER_NAME);
}


module_init(tablet_init);
module_exit(tablet_exit);


#endif // HUION_DRIVER_H