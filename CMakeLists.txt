# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(N684C_Ethernet_Card C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(N684C_Ethernet_Card N684C_Ethernet_Card.c
                enc28j60.c
                DataLink.c
                Network.c
                Transport.c
                operations.c
)

pico_set_program_name(N684C_Ethernet_Card "N684C_Ethernet_Card")
pico_set_program_version(N684C_Ethernet_Card "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(N684C_Ethernet_Card 1)
pico_enable_stdio_usb(N684C_Ethernet_Card 1)

# Add the standard library to the build
target_link_libraries(N684C_Ethernet_Card
        pico_stdlib
        pico_multicore
        pico_sync)

# Add the standard include files to the build
target_include_directories(N684C_Ethernet_Card PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(N684C_Ethernet_Card 
        hardware_spi
        )

pico_add_extra_outputs(N684C_Ethernet_Card)

