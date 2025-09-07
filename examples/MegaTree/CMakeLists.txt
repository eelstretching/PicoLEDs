# Generated Cmake Pico project file

pico_sdk_init()

add_executable(MegaTree MegaTree.cpp TripleColor.cpp Spiral.cpp LineFill.cpp FillLine.cpp)

pico_set_program_name(MegaTree "MegaTree")
pico_set_program_version(MegaTree "0.1")

pico_enable_stdio_uart(MegaTree 1)
pico_enable_stdio_usb(MegaTree 0)

# Add the standard include files to the build
target_include_directories(MegaTree PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(MegaTree 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
)

pico_add_extra_outputs(MegaTree)

