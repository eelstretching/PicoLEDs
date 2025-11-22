
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

file(GLOB FILES *.cpp ${PicoLEDsBuild_SOURCE_DIR/}/include/*.h)
add_library(PicoLEDs ${FILES})

# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(PicoLEDs ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio ${CMAKE_CURRENT_LIST_DIR}/ws2811.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Add the standard include files to the build
target_include_directories(PicoLEDs PUBLIC
  ../include
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. 
)
# target_sources(PicoLEDs PUBLIC ${FILES})

# Add any user requested libraries
target_link_libraries(PicoLEDs PUBLIC
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_runtime
  pico_stdlib
  pico_rand
)
