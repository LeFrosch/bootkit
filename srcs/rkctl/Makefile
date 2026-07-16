SRCS := main.c 
HDRS := $(wildcard *.h) $(wildcard ../include/*.h) ../build/uefi.h

# docker build #################################################################
BUILD_DIR  := build
MOUNT      := /build/application

DOCKER_RUN = docker run --rm --platform linux/amd64 \
	-v $(realpath .)/$(BUILD_DIR):$(MOUNT) \
	-w $(MOUNT) linux-base-amd64

docker-image:
	$(MAKE) -C $(realpath $(CURDIR)/../..) linux-base

$(BUILD_DIR)/rkctl: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 rkctl

# regular make build ###########################################################
CC          := gcc
CFLAGS 	    := -Wall -Wextra -Wpedantic -Wconversion -Werror -Wno-old-style-declaration -std=c23 -D_GNU_SOURCE

ifeq ($(DEBUG), 1)
CFLAGS += -DLOGGING -O0 -g
endif

%.o: %.c $(HDRS) uefi.h
	$(CC) $(CFLAGS) -c -o $@ $<

rkctl: $(SRCS:%.c=%.o) 
	$(CC) $^ -o $@

# housekeeping ##################################################################
clean:
	rm -rf $(BUILD_DIR)

all: $(BUILD_DIR)/rkctl

.PHONY: all clean
