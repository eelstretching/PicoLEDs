# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/stgreen/pico/pico-sdk")

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(PicoLEDs C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# Add executable. Default name is the project name, version 0.1

add_executable(PicoLEDs PicoLEDs.cpp Strip.cpp Canvas.cpp View.cpp StopWatch.cpp Animation.cpp)

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(PicoLEDs ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_set_program_name(PicoLEDs "PicoLEDs")
pico_set_program_version(PicoLEDs "0.1")

pico_enable_stdio_uart(PicoLEDs 1)
pico_enable_stdio_usb(PicoLEDs 0)

# Add the standard include files to the build
target_include_directories(PicoLEDs PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(PicoLEDs 
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_cyw43_arch_none
  pico_stdlib
)

pico_add_extra_outputs(PicoLEDs)

