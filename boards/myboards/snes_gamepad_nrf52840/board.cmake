# SPDX-License-Identifier: Apache-2.0

board_runner_args(pyocd "--target=nrf52840" "--frequency=8000000")
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
board_runner_args(openocd "-f interface/cmsis-dap.cfg" "-f target/nrf52.cfg")
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
board_runner_args(jlink "--device=nRF52840_xxAA" "--speed=4000")
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/uf2.board.cmake)