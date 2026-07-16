DEBUG     ?= 0
BUILD_DIR := build

export DEBUG

# nested builds ###############################################################
build-init:
	$(MAKE) -C init build/init

build-init.cpio:
	$(MAKE) -C init build/init.cpio

build-service:
	$(MAKE) -C uefi build/service.efi

build-loader: $(BUILD_DIR)/init.h $(BUILD_DIR)/service.h
	$(MAKE) -C uefi build/loader.efi

build-loader.iso: $(BUILD_DIR)/init.h $(BUILD_DIR)/service.h
	$(MAKE) -C uefi build/loader.iso

build-rkctl: $(BUILD_DIR)/loader.h
	$(MAKE) -C rkctl build/rkctl

# xxd headers #################################################################
$(BUILD_DIR)/init.h: build-init.cpio | $(BUILD_DIR)
	xxd -i -n init_archive init/build/init.cpio $@.tmp
	@cmp -s $@.tmp $@ || mv -f $@.tmp $@
	@rm -f $@.tmp

$(BUILD_DIR)/service.h: build-service | $(BUILD_DIR)
	xxd -i -n service_binary uefi/build/service.efi $@.tmp
	@cmp -s $@.tmp $@ || mv -f $@.tmp $@
	@rm -f $@.tmp

$(BUILD_DIR)/loader.h: build-loader | $(BUILD_DIR)
	xxd -i -n loader_binary uefi/build/loader.efi $@.tmp
	@cmp -s $@.tmp $@ || mv -f $@.tmp $@
	@rm -f $@.tmp

headers: $(BUILD_DIR)/init.h $(BUILD_DIR)/loader.h $(BUILD_DIR)/service.h

# housekeeping ################################################################
$(BUILD_DIR):
	@mkdir -p $@

all: headers build-init build-loader

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C init clean
	$(MAKE) -C loader clean
	$(MAKE) -C rkctl clean

.PHONY: all clean headers build-init build-init.cpio build-uefi build-uefi.iso
