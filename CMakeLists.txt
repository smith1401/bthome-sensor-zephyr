# #
# # Copyright (c) 2023 Nordic Semiconductor ASA
# #
# # SPDX-License-Identifier: Apache-2.0
# 
# This CMake file is picked by the Zephyr build system because it is defined
# as the module CMake entry point (see zephyr/module.yml).


set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# This is needed so that custom driver classes using system calls are taken into
# account
list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}
)
# zephyr_syscall_include_directories(include)
zephyr_include_directories(include)

add_subdirectory(lib)
