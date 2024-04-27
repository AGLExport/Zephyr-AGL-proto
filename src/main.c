#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

LOG_MODULE_REGISTER(main);

static const struct device *const uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

void main(void)
{
	int ret = -1;
	int result = 0;

	if (!device_is_ready(uart_dev)) {
		result = -1;
		goto do_return;
	}

	ret = usb_enable(NULL);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return 0;
	}

    while (1) {
		uart_poll_out (uart_dev, 'h');
		uart_poll_out (uart_dev, 'o');
		uart_poll_out (uart_dev, 'g');
		uart_poll_out (uart_dev, 'e');

        k_usleep(250 * 1000);
        //k_usleep(1);
    }
do_return:
	return;
}
