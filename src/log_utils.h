#include <linux/kernel.h>

#define LOG(x, ...) \
    "\033[1m\033[33m" "[Huion H640P]: " "\x1B[0m" x __VA_OPT__(,) __VA_ARGS__

#define LOG_INFO(x, ...) \
    printk(KERN_INFO LOG(x, __VA_ARGS__))

#define LOG_ERR(x, ...) \
    printk(KERN_ERR LOG(x, __VA_ARGS__))
