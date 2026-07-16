#include "misc.h"

#include <linux/module.h>

static int __init rk_init(void) {
    rk_info("Kernel module loaded\n");
    return 0;
}

static void __exit rk_exit(void) {
    rk_info("Kernel module un-loaded\n");
}

module_init(rk_init);
module_exit(rk_exit);

MODULE_LICENSE("GPL");
