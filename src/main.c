#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>

LOG_MODULE_REGISTER(main);

void main(void)
{
	int ret = -1;
	int result = 0;

    while (1) {

        k_usleep(250 * 1000);
    }
do_return:
	return;
}
