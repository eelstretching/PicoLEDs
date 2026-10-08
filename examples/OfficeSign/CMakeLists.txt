# OfficeSign talks Bluetooth through the CYW43 radio, so it can only be built
# for boards that have one (pico_w, pico2_w). Skip it for everything else.
if (NOT PICO_CYW43_SUPPORTED)
  message(STATUS "Skipping OfficeSign: board ${PICO_BOARD} has no CYW43 wireless chip")
  return()
endif()

pico_sdk_init()

add_executable(OfficeSign
    OfficeSign.cpp
    WireFormat.cpp
    SignMessage.cpp
    Mailbox.cpp
    BtReadySignal.cpp
    BluetoothServer.cpp
    ModeController.cpp
    ClockWeatherAnimation.cpp
    MeetingModeAnimation.cpp
)

pico_set_program_name(OfficeSign "OfficeSign")
pico_set_program_version(OfficeSign "0.1")

pico_enable_stdio_uart(OfficeSign 1)
pico_enable_stdio_usb(OfficeSign 0)

# btstack_config.h and our own headers.
target_include_directories(OfficeSign PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(OfficeSign
  PicoLEDs
  pico_cyw43_arch_none
  pico_btstack_cyw43
  pico_btstack_ble
  pico_multicore
  pico_aon_timer
  hardware_dma
  hardware_pio
  hardware_timer
  hardware_clocks
  pico_stdlib
  pico_rand
)

pico_btstack_make_gatt_header(OfficeSign PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/OfficeSign.gatt
)

pico_add_extra_outputs(OfficeSign)
