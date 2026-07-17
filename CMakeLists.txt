cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(picotemp C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(picotemp
  picotemp.c
  max6675.c
)



target_include_directories(picotemp PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)


target_link_libraries(picotemp 
  pico_stdlib
  hardware_spi
)

pico_enable_stdio_usb(picotemp 1)
pico_enable_stdio_uart(picotemp 1)

pico_add_extra_outputs(picotemp)
