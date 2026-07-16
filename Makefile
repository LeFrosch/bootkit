BUILD_DIR := build
BIN_DIR   := $(BUILD_DIR)/binaries
PKG_DIR   := $(BUILD_DIR)/packages
GDB_DIR   := $(BUILD_DIR)/gdb
MOD_DIR   := $(BUILD_DIR)/modules
EFI_DIR  := $(BUILD_DIR)/efi
KCONFIG   := config.amd64_none_amd64
DOCKER    := docker buildx build --platform linux/amd64 --build-arg KCONFIG=$(KCONFIG)

# kernel builds ###############################################################
$(BIN_DIR)/vmlinux: Dockerfile $(KCONFIG)
	$(DOCKER) --target vmlinux --output type=local,dest=$(BUILD_DIR) .

$(BIN_DIR)/bzImage: Dockerfile $(KCONFIG)
	$(DOCKER) --target bzimage --output type=local,dest=$(BUILD_DIR) .

$(PKG_DIR)/.stamp: Dockerfile $(KCONFIG) | $(PKG_DIR)
	$(DOCKER) --target deb --output type=local,dest=$(BUILD_DIR) . && touch $@

$(GDB_DIR)/vmlinux-gdb.py: Dockerfile $(KCONFIG)
	$(DOCKER) --target gdb --output type=local,dest=$(BUILD_DIR) .

$(BUILD_DIR)/.linux-base: Dockerfile $(KCONFIG) | $(BUILD_DIR)
	$(DOCKER) --target base --load -t linux-base-amd64 . && touch $@

$(BUILD_DIR)/.linux-modules: Dockerfile $(KCONFIG) | $(BUILD_DIR)
	$(DOCKER) --target build-modules --load -t linux-modules-amd64 . && touch $@

$(MOD_DIR)/.stamp: Dockerfile $(KCONFIG) | $(MOD_DIR)
	$(DOCKER) --target modules --output type=local,dest=$(BUILD_DIR) . && touch $@

linux-menuconfig: $(BUILD_DIR)/.linux-base.stamp
	docker run --platform linux/amd64 -it --rm -v $(PWD):/host linux-base-amd64 \
		sh -c "cp /host/$(KCONFIG) .config && make menuconfig && cp .config /host/$(KCONFIG)"

linux-shell: $(BUILD_DIR)/.linux-base
	docker run --platform linux/amd64 -it --rm -v $(PWD):/host linux-base-amd64 bash

linux-base: $(BUILD_DIR)/.linux-base
linux-modules: $(BUILD_DIR)/.linux-modules

# gnu-efi builds ##############################################################
$(EFI_DIR)/.stamp: Dockerfile $(EFI_DIR)
	$(DOCKER) --target efi --output type=local,dest=$(BUILD_DIR) . && touch $@

$(BUILD_DIR)/.gnu-efi: Dockerfile | $(BUILD_DIR)
	$(DOCKER) --target build-efi --load -t gnu-efi-amd64 . && touch $@

efi: $(BUILD_DIR)/.gnu-efi

# housekeeping ################################################################
$(BUILD_DIR) $(PKG_DIR) $(GDB_DIR) $(MOD_DIR) $(EFI_DIR):
	@mkdir -p $@

all:
	$(MAKE) -C srcs/uefi all
	$(MAKE) -C srcs/rkctl all
	$(MAKE) -C srcs/module all

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean linux-menuconfig linux-shell linux-base linux-modules efi
