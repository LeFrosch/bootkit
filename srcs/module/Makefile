SRCS := main.c 
HDRS := $(wildcard *.h) $(wildcard ../include/*.h)

# docker build ################################################################
BUILD_DIR  := build
KDIR	   := /build/linux
MOUNT      := /build/module

DOCKER_RUN = docker run --rm --platform linux/amd64 \
	-v $(realpath $(BUILD_DIR)):$(MOUNT) \
	-w $(MOUNT) linux-modules-amd64

docker-image:
	$(MAKE) -C $(realpath $(CURDIR)/../..) linux-modules

$(BUILD_DIR)/rootkit.ko: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 -C $(KDIR) M=$(MOUNT) modules

# kernel module build #########################################################
obj-m      := rootkit.o
rootkit-y  := $(SRCS:.c=.o)
ccflags-y  += -std=gnu17 -Wall -Wextra -Werror -Og -g

# housekeeping ################################################################
clean:
	rm -rf $(BUILD_DIR) 

all: $(BUILD_DIR)/rootkit.ko

.PHONY: all clean docker-image
