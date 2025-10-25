cmake_minimum_required(VERSION 3.17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

# pico SDK comes with a "download from git" option

include(FetchContent)

set(PICO_SDK_PATH pico-sdk)
set(PICOVGA_PATH ${CMAKE_CURRENT_LIST_DIR}/bin/picovga-cmake)

include(third_party/pico_sdk_import.cmake)
include(third_party/picovga_import.cmake)

project(piqro C CXX ASM)

pico_sdk_init()

add_compile_options(-Wall -Wno-format -Wno-unused-function -Wno-maybe-uninitialized)

add_executable(piqro
	src/main.cpp
)

add_picovga(piqro)

target_include_directories(piqro PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src
)

pico_add_extra_outputs(piqro)