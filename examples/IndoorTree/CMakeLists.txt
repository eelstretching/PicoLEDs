# Traditional Christmas lights for a real tree indoors: 4-5 strands of 200 on
# a Pico 2 W through an output board. See IndoorTree.cpp for the wiring.

pico_sdk_init()

add_executable(IndoorTree IndoorTree.cpp)
pico_set_program_name(IndoorTree "IndoorTree")
pico_enable_stdio_uart(IndoorTree 1)
pico_enable_stdio_usb(IndoorTree 1)
target_link_libraries(IndoorTree
  PicoLEDs
  pico_cyw43_arch_none
  pico_rand
  pico_stdlib
)
pico_add_extra_outputs(IndoorTree)
