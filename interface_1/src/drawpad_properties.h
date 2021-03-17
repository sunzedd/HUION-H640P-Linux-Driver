#ifndef __DRAWPAD_PROPERTIES__
#define __DRAWPAD_PROPERTIES__

#include <linux/input-event-codes.h>

#define VENDOR_ID       0x256c    // Huion Animation Co.
#define PRODUCT_ID      0x006d    // H640P Drawpad

#define MAX_PEN_PRESSURE            8191
#define MAX_PAD_RESOLUTION_VALUE    200
#define MAX_ABS_X                   32000
#define MAX_ABS_Y                   20000

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
};

#endif // __DRAWPAD_PROPERTIES__