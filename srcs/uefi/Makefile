SRCS := main.c 
HDRS := $(wildcard *.h) $(wildcard ../include/*.h)

# docker build ################################################################
BUILD_DIR  := build
EDIR       := /build/efi
MOUNT      := /build/application

DOCKER_RUN = docker run --rm --platform linux/amd64 \
	-v $(realpath .)/$(BUILD_DIR):$(MOUNT) \
	-w $(MOUNT) gnu-efi-amd64

docker-image:
	$(MAKE) -C $(realpath $(CURDIR)/../..) efi

$(BUILD_DIR)/app.efi: $(SRCS) $(HDRS) docker-image
	$(MAKE) -C ../init all
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)	
	@cp -f ../init/build/init.h $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 app.efi

$(BUILD_DIR)/efi.iso: $(SRCS) $(HDRS) docker-image
	$(MAKE) -C ../init all
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) Makefile $(BUILD_DIR)
	@cp -f ../init/build/init.h $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 efi.iso
	
# regular make build ##########################################################
CC          := gcc
CFLAGS 	    := -Wall -Wextra -Wconversion -Werror -Wno-old-style-declaration -std=c23 -fPIC -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -I$(EDIR)/inc 
LINKFLAGS   := -nostdlib -shared -Wl,-Bsymbolic -L$(EDIR)/x86_64/lib -L$(EDIR)/x86_64/gnuefi -Wl,-T,$(EDIR)/gnuefi/elf_x86_64_efi.lds $(EDIR)/x86_64/gnuefi/crt0-efi-x86_64.o
OBJFLAGS    := -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --output-target efi-app-x86_64 --subsystem=10

%.o: %.c $(HDRS) init.h
	$(CC) $(CFLAGS) $(EFIFLAGS) -c -o $@ $<

app.so: $(SRCS:%.c=%.o)
	$(CC) $(LINKFLAGS) $^ -lgnuefi -lefi -o $@

# efi boot image build ########################################################
app.efi: app.so
	objcopy $(OBJFLAGS) $< $@

efi.img: app.efi
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $< ::/EFI/BOOT/BOOTX64.EFI

efi.iso: efi.img
	@mkdir -p iso
	@cp $< iso
	xorriso -as mkisofs -R -f -e $< -no-emul-boot -o $@ iso

# housekeeping ################################################################
clean:
	rm -rf $(BUILD_DIR) 

all: $(BUILD_DIR)/app.efi $(BUILD_DIR)/efi.iso

.PHONY: all clean docker-image
