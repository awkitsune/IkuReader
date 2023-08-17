#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Set DEVKITARM in your environment.")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	iku
export TOPDIR		:=	$(CURDIR)
export PATH			:=	$(DEVKITARM)/bin:$(PATH)

# Mount point for media.
microSD			:=	/j/

#-------------------------------------------------------------------------------
# Device emulation.
#-------------------------------------------------------------------------------
# Location of desmume.
EMULATOR			:=	desmume-cli
# Compact flash image file for emulation.
CFLASH_IMAGE		:=	cflash.img
# Path to access mounted emulation image.
CFLASH_MOUNTPOINT	:=	./cflash

#-------------------------------------------------------------------------------
# path to tools
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Platform overrides.
#-------------------------------------------------------------------------------
##include Makefile.$(shell uname)

.PHONY: $(TARGET).arm7 $(TARGET).arm9

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all: $(TARGET).ds.gba
$(TARGET).ds.gba	: $(TARGET).nds

#-------------------------------------------------------------------------------
$(TARGET).nds		: $(TARGET).arm7 $(TARGET).arm9
	ndstool -b data/icon.bmp \
	"IkuReader;an ebook reader; " \
	-c sandbox/$(TARGET).nds -7 arm7/$(TARGET).arm7 -9 arm9/$(TARGET).arm9

#-------------------------------------------------------------------------------
$(TARGET).arm7		: arm7/$(TARGET).elf
$(TARGET).arm9		: arm9/$(TARGET).elf

#-------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7

#-------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#-------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f sandbox/$(TARGET).nds

copy_to_microSD:
	cp sandbox/$(TARGET).nds $(microSD)