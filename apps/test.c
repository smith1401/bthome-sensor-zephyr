#include <zephyr/kernel.h>

int main()
{
    // *(uint32_t *)0x10001008 = 0xFFFFFF00;

    for (;;)
    {
        printk("Hello World %d\r\n", k_uptime_get_32());
        k_msleep(1000);
    }
}