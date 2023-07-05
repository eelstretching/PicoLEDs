# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/stgreen/pico/pico-sdk")

set(PICO_MBEDTLS_PATH "/home/stgreen/pico/pico-sdk/lib/mbedtls")

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(../../pico_sdk_import.cmake)

if(PICO_SDK_VERSION_STRING VERSION_LESS "1.5.1")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.1 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(CampSign C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# Add executable. Default name is the project name, version 0.1
add_executable(CampSign ntp_client.c tls_http.c CampSign.cpp)

pico_set_program_name(CampSign "CampSign")
pico_set_program_version(CampSign "0.1")

pico_enable_stdio_uart(CampSign 1)
pico_enable_stdio_usb(CampSign 0)

# Add the standard include files to the build
target_include_directories(CampSign PRIVATE
  ../../include
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/..
)

target_compile_definitions(CampSign PRIVATE
  WIFI_SSID=\"${WIFI_SSID}\"
  WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

# Add any user requested libraries
target_link_libraries(CampSign
  PicoLEDs
  pico_cyw43_arch_lwip_threadsafe_background
  pico_lwip_mbedtls
  pico_mbedtls
  hardware_dma
  hardware_pio
  hardware_rtc
  hardware_timer
  hardware_clocks
  pico_stdlib
  pico_rand
)

pico_add_extra_outputs(CampSign)
