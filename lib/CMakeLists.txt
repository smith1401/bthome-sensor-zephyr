#
# Copyright (c) 2024 Schmied IFG
#
# SPDX-License-Identifier: Apache-2.0
#

add_subdirectory_ifdef(CONFIG_BTHOME bthome)
add_subdirectory_ifdef(CONFIG_SENSORS sensors)