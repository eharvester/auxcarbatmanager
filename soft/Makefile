# set the following variables 
# path to Arduino IDE
# PATH_TO_ARDUINO_IDE = /path/to/your/arduinoIDE
# Arduino IDE version
# ARDUINO_IDE_VER = 1.6.X
# local github mirror, if applicable, https://github.com is used per default
# GITHUB_MIRROR = /path/to/your/github mirror

ARDUINO_DIR = $(PATH_TO_ARDUINO_IDE)/arduino-$(ARDUINO_IDE_VER)

ifneq ("$(wildcard $(GITHUB_MIRROR))","")
GITHUB = $(GITHUB_MIRROR)
else
GITHUB = https://github.com
endif

.PHONY: default git_setup clean_all clean_builds fuse_read fuse_write


ISP_PROG = usbasp
MCU=atmega328p
F_CPU=16000000UL 
CORE=arduino
VARIANT=standard
HEX_MAXIMUM_SIZE=32256

BOARD_TAG    = AuxBatMonitor

# CPPFLAGS += -D$(CMD)

#default target
default: git_setup
######################################################################################
# setup section
#local library directories


USER_LIB_PATH := ./local-libs

ARDMK_DIR := $(USER_LIB_PATH)/Arduino-Makefile

ARDUINO_LIBS =  Low-Power Talkie/Talkie

LOWPWR_DIR := $(USER_LIB_PATH)/Low-Power 
TALKIE_DIR :=  $(USER_LIB_PATH)/Talkie

LOCAL_GIT_LIBRARIES = $(ARDMK_DIR) $(LOWPWR_DIR) $(TALKIE_DIR)

# local library includes
# Arduino Makefile environment
ifneq ("$(wildcard $(ARDMK_DIR))","")
include $(ARDMK_DIR)/Arduino.mk
endif

LUT_DIR = ./LUT

#setup part
git_setup: $(LOCAL_GIT_LIBRARIES)
	@echo Do the following:
	@echo make ispload

#local library target/rules
$(ARDMK_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/sudar/Arduino-Makefile $@

$(LOWPWR_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/rocketscream/Low-Power $@

$(TALKIE_DIR): 
	@rm -rf $@
	@git clone $(GITHUB)/going-digital/Talkie $@




temp.cpp: tempLUT.h

tempLUT.h: $(LUT_DIR)/tempLUTgen
	$(LUT_DIR)/tempLUTgen > tempLUT.h

$(LUT_DIR)/tempLUTgen: $(LUT_DIR)/tempLUTgen.c
	rm -rf $(LUT_DIR)/tempLUTgen
	gcc $(LUT_DIR)/tempLUTgen.c -lm -o $(LUT_DIR)/tempLUTgen


volts.cpp: voltsLUT.h

voltsLUT.h: $(LUT_DIR)/voltsLUTgen
	$(LUT_DIR)/voltsLUTgen > voltsLUT.h

$(LUT_DIR)/voltsLUTgen: $(LUT_DIR)/voltsLUTgen.c
	rm -rf $(LUT_DIR)/voltsLUTgen
	gcc $(LUT_DIR)/voltsLUTgen.c -lm -o $(LUT_DIR)/voltsLUTgen


#dependencies for main
main.ino: voicedata.h temp.cpp volts.cpp

######################################################################################
#cleanup section
clean_all: clean_builds 
	@echo "Cleaning local libs ..."
	@rm -rf ./local-libs
	@rm -rf ./build-*
	@rm -rf tempLUT.h
	@rm -rf $(LUT_DIR)/tempLUTgen
	@rm -rf voltsLUT.h
	@rm -rf $(LUT_DIR)/voltsLUTgen


clean_builds: 
	@echo "Cleaning builds ..."
	@rm -rf ./build-*
	@rm -rf tempLUT.h
	@rm -rf $(LUT_DIR)/tempLUTgen
	@rm -rf voltsLUT.h
	@rm -rf $(LUT_DIR)/voltsLUTgen



#other PHONY stuff
fuse_read:
	avrdude -p $(MCU) $(AVRDUDE_ISP_OPTS) -v
#16 MHz crystal
fuse_write: 
	avrdude -p $(MCU) $(AVRDUDE_ISP_OPTS) -v -U lfuse:w:0xC6:m -U hfuse:w:0xD2:m -U efuse:w:0xFD:m


