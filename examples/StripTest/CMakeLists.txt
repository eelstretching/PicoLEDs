# Generated Cmake Pico project file

pico_sdk_init()

add_executable(StripTest StripTest.cpp)
add_executable(StripTestSingle StripTestSingle.cpp)
add_executable(RenderTest RenderTest.cpp)

pico_set_program_name(StripTest "StripTest")
pico_set_program_version(StripTest "0.1")

pico_enable_stdio_uart(StripTest 1)
pico_enable_stdio_usb(StripTest 0)

# Add the standard include files to the build
target_include_directories(StripTest PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(StripTest 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
)

pico_add_extra_outputs(StripTest)

# Add the standard include files to the build
target_include_directories(StripTestSingle PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(StripTestSingle 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
)

pico_add_extra_outputs(StripTestSingle)

# Add the standard include files to the build
target_include_directories(RenderTest PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(RenderTest 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
)

pico_add_extra_outputs(RenderTest)
