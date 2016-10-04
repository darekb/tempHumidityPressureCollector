CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

PROJECT(cmake-avr)

#SET(MCU_SPEED "16000000")
SET(F_CPU "16000000UL")
SET(AVR_MCU "atmega328p")
set(AVR_H_FUSE 0xD6)
set(AVR_L_FUSE 0xF7)

set(AVR_UPLOADTOOL avrdude)
set(AVR_PROGRAMMER usbasp)
set(AVR_UPLOADTOOL_PORT usb)

set(CMAKE_BUILD_TYPE Release)

SET(SOURCE_EXE
        main.c
        slI2C.c
        slUart.c
        BME280.c
)

SET(SOURCE_LIB

)

include_directories(../common)

##########################################################################
# compiler options for all build types
##########################################################################
#add_definitions("-DF_CPU=${MCU_SPEED}")
add_definitions("-DF_CPU=${F_CPU}")
#add_definitions("-fpack-struct")
#add_definitions("-fshort-enums")
#add_definitions("-Wall")
add_definitions("-Werror")
#add_definitions("-pedantic")
#add_definitions("-pedantic-errors")
#add_definitions("-funsigned-char")
#add_definitions("-funsigned-bitfields")
#add_definitions("-ffunction-sections")
#add_definitions("-c")
add_definitions("-std=gnu99")
add_definitions("-mmcu=${AVR_MCU}")

##########################################################################

include(avr-gcc.cmake)