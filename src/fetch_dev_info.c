#include "fetch_dev_info.h"

void print_usb_interface_description(const struct usb_interface *interface) {

    struct usb_host_interface *interface_desc = interface->cur_altsetting;

    LOG_INFO("\tInterface number: %d\n", interface_desc->desc.bInterfaceNumber);
    LOG_INFO("\tEndpoints count: %d\n", interface_desc->desc.bNumEndpoints);

    for (int i = 0; i < interface_desc->desc.bNumEndpoints; i++) {

        struct usb_endpoint_descriptor *endpoint_desc = 
            &(interface_desc->endpoint[i].desc);

        LOG_INFO("\t\tEndpointDescriptor[%d] -> bEndPointAddress: 0x%02X\n",
                  i, endpoint_desc->bEndpointAddress);
    }
}

int get_drawpad_interface_type(const struct usb_interface *interface) {

    struct usb_host_interface *interface_desc = interface->cur_altsetting;
    
    int drawpad_interface_type = -1;

    switch (interface_desc->desc.bInterfaceNumber) {
        case 0: drawpad_interface_type = PEN; break;
        case 1: drawpad_interface_type = PAD; break;
        // handle default?
    }

    return drawpad_interface_type;
}