BZ_IMAGE  := ../../build/binaries/bzImage
EFI_IMAGE := ../../srcs/uefi/build/efi.iso
QEMU      := qemu-system-x86_64
DISK      := debian.qcow2
INITRD    := initrd.img-6.12.74+custom-amd64
SHARE_DIR := share

QEMU_FLAGS := \
	-drive "file=$(DISK),if=virtio,format=qcow2" \
	-nographic \
	-m 8G \
	-netdev user,id=net0,hostfwd=tcp::4444-:4444,hostfwd=tcp::4445-:22 \
	-device virtio-net-pci,netdev=net0 \
	-fsdev local,id=share,path=$(SHARE_DIR),security_model=mapped-xattr \
	-device virtio-9p-pci,fsdev=share,mount_tag=share

# os specific configuration #####################################################
UNAME := $(shell uname -s)

ifeq ($(UNAME), Darwin)
	EDK2_FW    := /opt/homebrew/Cellar/qemu/11.0.1/share/qemu/edk2-x86_64-code.fd
	QEMU_FLAGS += -drive "format=raw,file=$(EDK2_FW),if=pflash,readonly=on" -machine q35
else ifeq ($(UNAME), Linux)
	EDK2_FW    := /usr/share/OVMF/OVMF_CODE.fd
	QEMU_FLAGS += -drive "format=raw,file=$(EDK2_FW),if=pflash,readonly=on" -machine q35 -enable-kvm 
else
	$(error Unsupported platform: $(UNAME))
endif

# bridge to other builds ########################################################
$(BZ_IMAGE): FORCE
	$(MAKE) -C ../../ build/binaries/bzImage

$(EFI_IMAGE): FORCE
	$(MAKE) -C ../../srcs/uefi build/efi.iso

# qemu interactive targets ######################################################
qemu-custom: $(BZ_IMAGE)
	$(QEMU) $(QEMU_FLAGS) -s \
		-kernel $(BZ_IMAGE) \
		-initrd $(INITRD) \
		-append "root=/dev/vda2 console=ttyS0"

qemu-default:
	$(QEMU) $(QEMU_FLAGS)

qemu-uefi: $(EFI_IMAGE)
	$(QEMU) $(QEMU_FLAGS) -drive file=$(EFI_IMAGE),format=raw,if=none,id=iso -device ide-cd,drive=iso,bootindex=1

qemu-ephermal:
	$(QEMU) $(QEMU_FLAGS) -snapshot

# housekeeping ################################################################
FORCE: # forces a target to rebuild every time

.PHONY: qemu-custom qemu-default qemu-uefi qemu-ephermal FORCE
