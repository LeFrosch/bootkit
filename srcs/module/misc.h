#pragma once

/* Enable C23 style type inference in C17 using GNU extensions. */
#define auto __auto_type

#ifdef DEBUG
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define rk_debug(fmt, ...) pr_debug(fmt, ##__VA_ARGS__)
#define rk_info(fmt, ...) pr_info(fmt, ##__VA_ARGS__)
#define rk_warn(fmt, ...) pr_warn(fmt, ##__VA_ARGS__)
#define rk_err(fmt, ...) pr_err(fmt, ##__VA_ARGS__)
#define rk_alert(fmt, ...) pr_alert(fmt, ##__VA_ARGS__)
#define rk_emerg(fmt, ...) pr_emerg(fmt, ##__VA_ARGS__)
#else
#define rk_debug(fmt, ...) no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#define rk_info(fmt, ...) no_printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define rk_warn(fmt, ...) no_printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define rk_err(fmt, ...) no_printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define rk_alert(fmt, ...) no_printk(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define rk_emerg(fmt, ...) no_printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#endif

/* Include after definifn pr_fmt. */
#include <linux/printk.h>
