SRCS := $(wildcard *.c)
HDRS := $(wildcard *.h) $(wildcard ../include/*.h)

# docker build ################################################################
BUILD_DIR  := build
MOUNT      := /build/application

DOCKER_RUN = docker run --rm --platform linux/amd64 \
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

$(BUILD_DIR)/init.h: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 init.h

# regular make build ###########################################################
CC          := musl-gcc
CFLAGS 	    := -Wall -Wextra -Wpedantic -Wconversion -Werror -Wno-old-style-declaration -std=c23 -Og -g 

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

init: $(SRCS:%.c=%.o) 
	$(CC) -static $^ -o $@

init.cpio: init
	@mkdir -p cpio
	@cp $< cpio
	(cd cpio && find . | cpio -o -H newc) > $@

init.h: init.cpio
	xxd -i -n init_archive $< $@

# housekeeping ################################################################
clean:
	rm -rf $(BUILD_DIR) 

all: $(BUILD_DIR)/init $(BUILD_DIR)/init.h

.PHONY: all clean docker-image
