#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Set DEVKITARM in your environment.")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)
export PATH			:=	$(DEVKITARM)/bin:$(PATH)

GAME_TITLE := IkuReader
GAME_SUBTITLE1 := An ebook reader
GAME_SUBTITLE2 := github.com/awkitsune/IkuReader
GAME_ICON := data/icon.bmp

#-------------------------------------------------------------------------------
# path to tools
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Platform overrides.
#-------------------------------------------------------------------------------
##include Makefile.$(shell uname)

.PHONY: checkarm7 checkarm9 clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkarm7 checkarm9 $(TARGET).nds

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#-------------------------------------------------------------------------------
$(TARGET).nds:
	$(SILENTCMD)ndstool -c sandbox/$(TARGET).nds -7 arm7/arm7.elf -9 arm9/arm9.elf \
		-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)"

	$(SILENTCMD)ndstool -c sandbox/$(TARGET).dsi -7 arm7/arm7.elf -9 arm9/arm9.elf \
		-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" \
		-g IKUR 01 "IKUREADER" $(VERSION) -z 80040000 -u 00030004 

	echo built ... $(notdir $@)

#-------------------------------------------------------------------------------
#(TARGET).arm7		: arm7/$(TARGET).elf
#$(TARGET).arm9		: arm9/$(TARGET).elf

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
	rm -f sandbox/$(TARGET).dsi