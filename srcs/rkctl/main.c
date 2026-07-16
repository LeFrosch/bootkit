#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "install.h"
#include "inspect.h"
#include "utils.h"

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        error("usage: [install|inspect]", -EINVAL);
    }

    if (strcmp(argv[1], "install") == 0) {
        rk_install();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "inspect") == 0) {
        rk_inspect();
        return EXIT_SUCCESS;
    }

    error("unknown argument", -EINVAL);
}
