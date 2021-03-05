#ifndef __HUION_DRAWPAD_LOG_UTILS__
#define __HUION_DRAWPAD_LOG_UTILS__

#include <linux/kernel.h>

#define LOG(x, ...) \
    "\033[1m\033[33m" "[Huion H640P]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__

#define LOG_PEN(x, ...) \
    "\033[0;32m" "[Huion H640P Pen]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__

#define LOG_PAD(x, ...) \
    "\033[0;35m" "[Huion H640P Pad]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__


#define LOG_INFO(x, ...) \
    printk(KERN_INFO LOG(x, __VA_ARGS__))

#define LOG_INFO_PEN(x, ...) \
    printk(KERN_INFO LOG_PEN(x, __VA_ARGS__))

#define LOG_INFO_PAD(x, ...) \
    printk(KERN_INFO LOG_PAD(x, __VA_ARGS__))


#define LOG_ERR(x, ...) \
    printk(KERN_ERR LOG(x, __VA_ARGS__))

#define LOG_ERR_PEN(x, ...) \
    printk(KERN_ERR LOG_PEN(x, __VA_ARGS__))

#define LOG_ERR_PAD(x, ...) \
    printk(KERN_ERR LOG_PAD(x, __VA_ARGS__))

#endif // __HUION_DRAWPAD_LOG_UTILS__