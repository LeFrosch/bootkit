DEBUG     ?= 0
BUILD_DIR := build

export DEBUG

# nested builds ###############################################################
build-init:
	$(MAKE) -C init build/init

build-init.cpio:
	$(MAKE) -C init build/init.cpio

build-uefi: $(BUILD_DIR)/init.h
	$(MAKE) -C uefi build/app.efi

build-uefi.iso: $(BUILD_DIR)/init.h
	$(MAKE) -C uefi build/efi.iso

build-rkctl: $(BUILD_DIR)/uefi.h
	$(MAKE) -C rkctl build/rkctl

# xxd headers #################################################################
$(BUILD_DIR)/init.h: build-init.cpio | $(BUILD_DIR)
	xxd -i -n init_archive init/build/init.cpio $@.tmp
	@cmp -s $@.tmp $@ || mv -f $@.tmp $@
	@rm -f $@.tmp

$(BUILD_DIR)/uefi.h: build-uefi | $(BUILD_DIR)
	xxd -i -n uefi_archive uefi/build/app.efi $@.tmp
	@cmp -s $@.tmp $@ || mv -f $@.tmp $@
	@rm -f $@.tmp

header: $(BUILD_DIR)/init.h $(BUILD_DIR)/uefi.h

# housekeeping ################################################################
$(BUILD_DIR):
	@mkdir -p $@

all: header build-init build-uefi

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C init clean
	$(MAKE) -C uefi clean
	$(MAKE) -C rkctl clean

.PHONY: all clean build-init build-init.cpio build-uefi build-uefi.iso
