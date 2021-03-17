#ifndef __HUION_DRAWPAD_LOG_UTILS__
#define __HUION_DRAWPAD_LOG_UTILS__

#include <linux/kernel.h>

#define LOG(x, ...) \
    "\033[0;35m" "[Huion H640P Interface 1]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__

// Журналирование уровня INFO
#define LOG_INFO(x, ...) \
    printk(KERN_INFO LOG(x, __VA_ARGS__))

// Журналирование уровня WARNING
#define LOG_WARN(x, ...) \
    printk(KERN_WARNING LOG(x, __VA_ARGS__))

// Журналирование уровня ERROR
#define LOG_ERR(x, ...) \
    printk(KERN_ERR LOG(x, __VA_ARGS__))

#endif // __HUION_DRAWPAD_LOG_UTILS__