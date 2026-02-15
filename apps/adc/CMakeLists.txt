cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(adc)

# Use GLOB to select all .c files in the src directory
file(GLOB SRC_FILES "src/*.c")

# Add the .c files to the target
target_sources(app PRIVATE ${SRC_FILES})