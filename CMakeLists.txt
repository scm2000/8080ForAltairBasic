# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PROJECT_NAME i8080ForAltairBASIC)
set(PICO_PLATFORM rp2040)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(i8080ForAltairBASIC C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1

add_executable(i8080ForAltairBASIC
        runBasic.c
        i8080.c
        i8080.h
        drivers/audio.c
        drivers/audio.h
        drivers/clib.c
        drivers/display.c
        drivers/display.h
        drivers/fat32.c
        drivers/fat32.h
        drivers/font-5x10.c
        drivers/font-8x10.c
        drivers/font.h
        drivers/keyboard.c
        drivers/keyboard.h
        drivers/lcd.c
        drivers/lcd.h
        drivers/onboard_led.c
        drivers/onboard_led.h
        drivers/picocalc.c
        drivers/picocalc.h
        drivers/sdcard.c
        drivers/sdcard.h
        drivers/southbridge.c
        drivers/southbridge.h
        )

        pico_set_program_name(i8080ForAltairBASIC "i8080ForAltairBASIC")
        pico_set_program_version(i8080ForAltairBASIC "0.10")


# Generate PIO header
pico_generate_pio_header(i8080ForAltairBASIC ${CMAKE_CURRENT_LIST_DIR}/drivers/audio.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(i8080ForAltairBASIC 0)
pico_enable_stdio_usb(i8080ForAltairBASIC 0)

# Turn on all warnings
target_compile_options(i8080ForAltairBASIC PRIVATE -Wall -Werror)

# Add the standard library to the build
target_link_libraries(i8080ForAltairBASIC
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(i8080ForAltairBASIC PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(i8080ForAltairBASIC
        pico_stdlib
        pico_printf
        pico_float
        pico_status_led
        pico_rand
        hardware_gpio
        hardware_i2c
        hardware_spi
        hardware_pio
        hardware_clocks
        )

        pico_add_extra_outputs(i8080ForAltairBASIC)

function(enable_sdcard_app target)
  if(${PICO_PLATFORM} STREQUAL "rp2040")
    pico_set_linker_script(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_default_rp2040.ld)
  elseif(${PICO_PLATFORM} MATCHES "rp2350")
    pico_set_linker_script(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_default_rp2350.ld)
  endif()
endfunction()

enable_sdcard_app(${CMAKE_PROJECT_NAME})

