# Generated Cmake Pico project file

pico_sdk_init()

add_executable(AniTest AniTest.cpp)

pico_set_program_name(AniTest "AniTest")
pico_set_program_version(AniTest "0.1")

pico_enable_stdio_uart(AniTest 1)
pico_enable_stdio_usb(AniTest 0)

# Add the standard include files to the build
target_include_directories(AniTest PRIVATE
  ../../src
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(AniTest 
  PicoLEDs
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
)

pico_add_extra_outputs(AniTest)

