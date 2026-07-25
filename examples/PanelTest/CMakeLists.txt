# Generated Cmake Pico project file

pico_sdk_init()

add_executable(PanelTest PanelTest.cpp)
  
pico_set_program_name(PanelTest "PanelTest")
pico_set_program_version(PanelTest "0.1")

pico_enable_stdio_uart(PanelTest 1)
pico_enable_stdio_usb(PanelTest 0)

# Add the standard include files to the build
target_include_directories(PanelTest PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(PanelTest 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
  pico_status_led
)

pico_add_extra_outputs(PanelTest)

