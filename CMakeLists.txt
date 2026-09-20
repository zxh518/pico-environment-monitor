# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(blink C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add DHT library
add_library(dht lib/dht.c)
target_include_directories(dht PUBLIC lib)
target_link_libraries(dht pico_stdlib hardware_sync)

# Add LCD Library
add_library(lcd_i2c lib/lcd_i2c.c)
target_include_directories(lcd_i2c PUBLIC lib)
target_link_libraries(lcd_i2c pico_stdlib hardware_i2c)

# Add executable
add_executable(blink blink.c)

pico_set_program_name(blink "blink")
pico_set_program_version(blink "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(blink 0)
pico_enable_stdio_usb(blink 1)

# Add the standard library to the build
target_link_libraries(blink
        pico_stdlib
        pico_cyw43_arch_none
        dht
        lcd_i2c
        hardware_i2c)

# Add the standard include files to the build
target_include_directories(blink PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(blink)