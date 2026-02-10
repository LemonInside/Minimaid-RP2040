cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(minimaid_clone)
pico_sdk_init()

add_executable(minimaid_clone main.c usb_descriptors.c)

# Add tinyusb device and hid support
target_link_libraries(minimaid_clone pico_stdlib tinyusb_device tinyusb_board)
target_include_directories(minimaid_clone PRIVATE ${CMAKE_CURRENT_LIST_DIR})
pico_enable_stdio_usb(minimaid_clone 0)
pico_add_extra_outputs(minimaid_clone)