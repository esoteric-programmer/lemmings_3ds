MKDIR = mkdir -p

# TARGET #

TARGET := 3DS
LIBRARY := 0

ifeq ($(TARGET),3DS)
    ifeq ($(strip $(DEVKITPRO)),)
        $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
    endif

    ifeq ($(strip $(DEVKITARM)),)
        $(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
    endif
endif

# COMMON CONFIGURATION #

NAME := lemmings

BUILD_DIR := build
OUTPUT_DIR := output
INCLUDE_DIRS := include
SOURCE_DIRS := src

EXTRA_OUTPUT_FILES :=

LIBRARY_DIRS := $(DEVKITPRO)/libctru
LIBRARIES := sf2d citro3d ctru

BUILD_FLAGS :=
RUN_FLAGS :=

VERSION_MAJOR := 0
VERSION_MINOR := 4
VERSION_MICRO := 0

# 3DS CONFIGURATION #

TITLE := Lemmings for 3DS
LONGTITLE := Lemmings for 3DS
DESCRIPTION := Clone of classic game
AUTHOR := Matthias
PRODUCT_CODE := CTR-P-CLEM
UNIQUE_ID := 0xF9173

SYSTEM_MODE := 64MB
SYSTEM_MODE_EXT := Legacy

ICON_FLAGS := --flags visible,ratingrequired,recordusage --cero 153 --esrb 153 --usk 153 --pegigen 153 --pegiptr 153 --pegibbfc 153 --cob 153 --grb 153 --cgsrr 153

ROMFS_DIR := romfs
BANNER_AUDIO := banner.wav
BANNER_IMAGE := banner.png
ICON := icon.png

# INTERNAL #

include buildtools/make_base

# ADD games/* AND TEXT FILES

TEXTFILES := $(wildcard *.txt) $(wildcard *.md)
TEXTFILES_OUTPUT := $(addprefix $(OUTPUT_DIR)/,$(TEXTFILES))
GAMES := $(wildcard games/*)
GAMES_OUTPUT := $(GAMES:games/%=$(OUTPUT_DIR)/lemmings/%)

$(OUTPUT_ZIP_FILE): $(GAMES_OUTPUT) $(TEXTFILES_OUTPUT)

$(OUTPUT_DIR)/lemmings/%: games/%
	@mkdir -p $(OUTPUT_DIR)/lemmings/
	@cp -r $< $@

$(OUTPUT_DIR)/%.txt: %.txt
	@cp $< $@

$(OUTPUT_DIR)/%.md: %.md
	@cp $< $@
