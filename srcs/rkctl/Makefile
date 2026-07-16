SRCS := main.c 
HDRS := $(wildcard *.h) $(wildcard ../include/*.h)

# docker build #################################################################
BUILD_DIR  := build
MOUNT      := /build/rkctl

DOCKER_RUN = docker run --rm --platform linux/amd64 \
	-v $(realpath $(CURDIR)):$(MOUNT) \
	-w $(MOUNT) linux-base-amd64

docker-image:
	$(MAKE) -C $(realpath $(CURDIR)/../..) linux-base

$(BUILD_DIR)/rkctl: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make rkctl

# regular make build ###########################################################
CC          := gcc
CFLAGS 	    := -Wall -Wextra -Wpedantic -Wconversion -Werror -Wno-old-style-declaration -std=c23 -D_GNU_SOURCE -Og -g 

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

rkctl: $(SRCS:%.c=%.o) 
	$(CC) $^ -o $@

# housekeeping ##################################################################
clean:
	rm -rf $(BUILD_DIR)

all: $(BUILD_DIR)/rkctl

.PHONY: all clean
