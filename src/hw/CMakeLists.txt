cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT on)

include(pico_sdk_import.cmake)

pico_sdk_init()

project(piqro C CXX)

add_executable(piqro
    main.c
)

pico_enable_stdio_usb(piqro 1)
pico_enable_stdio_uart(piqro 0)

target_link_libraries(piqro 
    pico_stdlib
    hardware_spi
    hardware_dma
)

target_include_directories(piqro PRIVATE
    ..)

pico_add_extra_outputs(piqro)