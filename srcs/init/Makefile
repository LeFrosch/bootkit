SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h) $(wildcard ../shared/*.h)

# docker build ################################################################
BUILD_DIR  := build
MOUNT      := /build/application

DOCKER_RUN = docker run --rm --platform linux/amd64 \
	-e DEBUG=$(DEBUG) \
	-v $(realpath .)/$(BUILD_DIR):$(MOUNT) \
	-w $(MOUNT) linux-base-amd64

docker-image:
	$(MAKE) -C $(realpath $(CURDIR)/../..) linux-base

$(BUILD_DIR)/init: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 init

$(BUILD_DIR)/init.cpio: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 init.cpio

# regular make build ###########################################################
CC          := musl-gcc
CFLAGS 	    := -Wall -Wextra -Wpedantic -Wconversion -Werror -Wno-old-style-declaration -std=c23 -D_GNU_SOURCE

ifeq ($(DEBUG), 1)
CFLAGS += -DLOGGING -O0 -g
endif

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

init: $(SRCS:%.c=%.o) 
	$(CC) -static $^ -o $@

init.cpio: init
	# create a clean cpio dir
	@rm -rf cpio
	@mkdir -p cpio
	
	# copy the binary as init-helper (innocent name, wink)
	@cp $< cpio/init-helper

	# use symlinks to replace the busybox hardlinks (this avoids overwriting the actual binary)
	@mkdir -p cpio/usr/sbin
	@ln -s /init-helper cpio/usr/sbin/run-init
	@mkdir -p cpio/usr/bin
	@ln -s /init-helper cpio/usr/bin/run-init

	# actually create the cpio archive
	(cd cpio && find . | cpio -o -H newc | gzip -9) > $@

# housekeeping ################################################################
clean:
	rm -rf $(BUILD_DIR) 

all: $(BUILD_DIR)/init $(BUILD_DIR)/init.cpio

.PHONY: all clean docker-image
