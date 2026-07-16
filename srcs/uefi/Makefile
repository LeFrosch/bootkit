SRCS := $(filter-out loader.c service.c, $(wildcard *.c))
HDRS := $(wildcard *.h) $(wildcard ../shared/*.h) ../build/init.h ../build/service.h

# docker build ################################################################
BUILD_DIR  := build
EDIR       := /build/efi
MOUNT      := /build/application

DOCKER_RUN = docker run --rm --platform linux/amd64 \
	-e DEBUG=$(DEBUG) \
	-v $(realpath .)/$(BUILD_DIR):$(MOUNT) \
	-w $(MOUNT) gnu-efi-amd64

docker-image:
	$(MAKE) -C $(realpath $(CURDIR)/../..) efi

$(BUILD_DIR)/service.efi: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) service.c Makefile $(BUILD_DIR)	
	$(DOCKER_RUN) make -j8 service.efi

$(BUILD_DIR)/loader.efi: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) loader.c Makefile $(BUILD_DIR)	
	$(DOCKER_RUN) make -j8 loader.efi

$(BUILD_DIR)/loader.iso: $(SRCS) $(HDRS) docker-image
	@mkdir -p $(BUILD_DIR)
	@cp -f -r $(SRCS) $(HDRS) loader.c Makefile $(BUILD_DIR)
	$(DOCKER_RUN) make -j8 loader.iso
	
# regular make build ##########################################################
CC          := gcc
CFLAGS 	    := -Wall -Wextra -Wconversion -Werror -Wno-old-style-declaration -std=c23 -fPIC -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -I$(EDIR)/inc 
LINKFLAGS   := -nostdlib -shared -Wl,-Bsymbolic -L$(EDIR)/x86_64/lib -L$(EDIR)/x86_64/gnuefi -Wl,-T,$(EDIR)/gnuefi/elf_x86_64_efi.lds $(EDIR)/x86_64/gnuefi/crt0-efi-x86_64.o
OBJFLAGS    := -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --output-target efi-app-x86_64

ifeq ($(DEBUG), 1)
CFLAGS += -DLOGGING -O0 -g
endif 

%.o: %.c $(HDRS) init.h service.h
	$(CC) $(CFLAGS) $(EFIFLAGS) -c -o $@ $<

loader.so: $(SRCS:%.c=%.o) loader.o
	$(CC) $(LINKFLAGS) $^ -lgnuefi -lefi -o $@

service.so: $(SRCS:%.c=%.o) service.o
	$(CC) $(LINKFLAGS) $^ -lgnuefi -lefi -o $@

# efi boot image build ########################################################
loader.efi: loader.so
	objcopy $(OBJFLAGS) --subsystem=10 $< $@

service.efi: service.so
	objcopy $(OBJFLAGS) --subsystem=12 $< $@

loader.img: loader.efi
	dd if=/dev/zero of=$@ bs=1k count=1440
	mformat -i $@ -f 1440 ::
	mmd -i $@ ::/EFI
	mmd -i $@ ::/EFI/BOOT
	mcopy -i $@ $< ::/EFI/BOOT/BOOTX64.EFI

loader.iso: loader.img
	@mkdir -p iso
	@cp $< iso
	xorriso -as mkisofs -R -f -e $< -no-emul-boot -o $@ iso

# housekeeping ################################################################
clean:
	rm -rf $(BUILD_DIR) 

all: $(BUILD_DIR)/service.efi $(BUILD_DIR)/loader.efi $(BUILD_DIR)/loader.iso

.PHONY: all clean docker-image
