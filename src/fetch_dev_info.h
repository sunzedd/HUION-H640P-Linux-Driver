#ifndef __HUION_DRAWPAD_FETCH_DEV_INFO__
#define __HUION_DRAWPAD_FETCH_DEV_INFO__

#include <linux/usb.h>
#include "log_utils.h"

enum DRAWPAD_INTERFACE {
    PEN = 0,
    PAD = 1
};

void print_usb_interface_description(const struct usb_interface *interface);
int get_drawpad_interface_type(const struct usb_interface *interface);

#endif // __HUION_DRAWPAD_FETCH_DEV_INFO__