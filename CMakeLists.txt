cmake_minimum_required(VERSION 3.10)
project(ipc)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(ipc
        src/main.c
        src/core/adapter_info.c
        src/core/adapter_utils.c
        src/display/display.c
        src/display/color_output.c
        src/utils/net_utils.c
        src/utils/string_utils.c
        src/utils/commands.c
        src/utils/ip_calculator.c
)

target_link_libraries(ipc
        ws2_32
        iphlpapi
        winhttp
)
