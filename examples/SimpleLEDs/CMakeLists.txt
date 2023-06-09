# Generated Cmake Pico project file

pico_sdk_init()

add_executable(SimpleLEDs SimpleLEDs.cpp)

pico_set_program_name(SimpleLEDs "SimpleLEDs")
pico_set_program_version(SimpleLEDs "0.1")

pico_enable_stdio_uart(SimpleLEDs 1)
pico_enable_stdio_usb(SimpleLEDs 0)

# Add the standard include files to the build
target_include_directories(SimpleLEDs PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(SimpleLEDs 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
)

pico_add_extra_outputs(SimpleLEDs)

