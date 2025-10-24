cmake_minimum_required(VERSION 4.0)

project(piqro C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(PICO_SDK_PATH pico_sdk)
set(PICOVGA_PATH picovga)

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