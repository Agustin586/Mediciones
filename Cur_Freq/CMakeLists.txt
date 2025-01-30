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
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Cur_Freq C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Collect all .c files in src directory
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)

# Add executable. Default name is the project name, version 0.1
add_executable(Cur_Freq ${SRC_FILES})

pico_set_program_name(Cur_Freq "Cur_Freq")
pico_set_program_version(Cur_Freq "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Cur_Freq 0)
pico_enable_stdio_usb(Cur_Freq 1)

# Add the standard library to the build
target_link_libraries(Cur_Freq
        pico_stdlib)

# Include all .h files from the include directory
target_include_directories(Cur_Freq PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include/
)

# Add any user requested libraries
target_link_libraries(Cur_Freq 
        hardware_i2c
        hardware_timer
        hardware_clocks
        pico_multicore
)

pico_add_extra_outputs(Cur_Freq)