#ifndef __HUION_DRAWPAD_LOG_UTILS__
#define __HUION_DRAWPAD_LOG_UTILS__

#include <linux/kernel.h>


#define LOG_INTF_0(x, ...) \
    "\033[0;32m" "[Huion H640P Interface 0]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__

#define LOG_INTF_1(x, ...) \
    "\033[0;35m" "[Huion H640P Interface 1]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__


// Журналирование уровня INFO
#define LOG_INFO_INTF_0(x, ...) \
    printk(KERN_INFO LOG_INTF_0(x, __VA_ARGS__))

#define LOG_INFO_INTF_1(x, ...) \
    printk(KERN_INFO LOG_INTF_1(x, __VA_ARGS__))


// Журналирование уровня WARNING
#define LOG_WARN_INTF_0(x, ...) \
    printk(KERN_WARNING LOG_INTF_0(x, __VA_ARGS__))

#define LOG_WARN_INTF_1(x, ...) \
    printk(KERN_WARNING LOG_INTF_1(x, __VA_ARGS__))


// Журналирование уровня ERROR
#define LOG_ERR_INTF_0(x, ...) \
    printk(KERN_ERR LOG_INTF_0(x, __VA_ARGS__))

#define LOG_ERR_INTF_1(x, ...) \
    printk(KERN_ERR LOG_INTF_1(x, __VA_ARGS__))

#endif // __HUION_DRAWPAD_LOG_UTILS__