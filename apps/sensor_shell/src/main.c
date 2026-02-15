/*
 * Copyright (c) 2018 Diego Sueiro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * This sample app launches a shell. Interact with it using the `sensor` command. See
 * `drivers/sensor/sensor_shell.c`. There is nothing to do in the main thread.
 */

#include <zephyr/shell/shell.h>

int main(void)
{
	const struct shell *sh = shell_backend_rtt_get_ptr();

	shell_execute_cmd(sh, "i2c scan i2c@40003000");
	return 0;
}
