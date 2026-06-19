cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32-smart-sensor-trash-can-f103c8t6)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Output map file
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wa,-adhlns=${CMAKE_PROJECT_NAME}.lst -Wl,-Map=${CMAKE_PROJECT_NAME}.map")

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Append suffix according to build type
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME "${CMAKE_PROJECT_NAME}_$<CONFIG>"
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Os
    -fdata-sections
    -ffunction-sections
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wl,--gc-sections 
    -Wl,--print-memory-usage
    --specs=nosys.specs
    # -u _printf_float  # highly increase code size
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/common.c
    Core/Src/debug.c
    Core/Src/bsp_hcsr04.c
    Core/Src/bsp_sg90.c
    Core/Src/bsp_eeprom.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

set(DEFAULT_DEFINES
    HCSR04_TRIG_INTERVAL=70
    LID_CLOSE_ANGLE=0
    LID_OPEN_ANGLE=90
    LID_HOLD_TIME=5000
    LID_CLOSING_TIME=1000
)

set(DEBUG_DEFINES
    ENABLE_UART_DEBUG=1
    CONFIG_UART_DEBUG_LEVEL=4
    CONFIG_UART_DEBUG_ONLY_FILE_NAME=1
    ENABLE_DEBUG_EEPROM=1
    ENABLE_DEBUG_SG90=0
    ENABLE_DEBUG_LED=0
)

set(RELEASE_DEFINES
    ENABLE_UART_DEBUG=0
    ENABLE_DEBUG_EEPROM=0
    ENABLE_DEBUG_SG90=0
    ENABLE_DEBUG_LED=0
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    ${DEFAULT_DEFINES}
    $<$<CONFIG:Debug>:${DEBUG_DEFINES}>
    $<$<NOT:$<CONFIG:Debug>>:${RELEASE_DEFINES}>
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
