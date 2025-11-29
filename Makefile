ARDUINO_DIR=/Users/dvaldespino94/Library/Arduino15/packages/arduino
ARDMK_DIR=/usr/local/Cellar/arduino-mk/1.6.0
AVR_TOOLS_DIR=/Users/dvaldespino94/Library/Arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7
ARDUINO_LIBS=EEPROM IRRemoteControl

CXXFLAGS += -Iinclude -Ipb
CFLAGS += -Iinclude -Ipb

BOARD_TAG    = pro
BOARD_SUB = 16MHzatmega328
include $(ARDMK_DIR)/Arduino.mk
