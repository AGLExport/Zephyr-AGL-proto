#include "io-adc.h"
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>

//-------------------------------------------------------------
LOG_MODULE_REGISTER(io_adc);

//-------------------------------------------------------------
// Global
// CAN device
const struct device *const g_adc_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_adc));

static const struct adc_channel_cfg ch_cfg = {
	.gain = ADC_GAIN_1,
	.reference = ADC_REF_INTERNAL,
	.acquisition_time = ADC_ACQ_TIME_DEFAULT,
	.channel_id = 0,
	.differential = false,
};

static uint16_t g_buf[4];
static struct adc_sequence sequence0 = {
	.buffer = &g_buf[3],
	.buffer_size = sizeof(g_buf[3]),
	.channels = 1 << 0,	//Ch.0
	.resolution = 12,	// 12bit AD
	.calibrate = false,
	.oversampling = false,
	.options = NULL,
};

//-------------------------------------------------------------
static int io_adc_init(const struct device *const adc_dev)
{
	int ret = -1;
	int result = 0;

	if (!device_is_ready(adc_dev)) {
		result = -1;
		goto do_return;
	}

	ret = adc_channel_setup(adc_dev, &ch_cfg);
	if (ret < 0) {
		LOG_ERR("adc_channel_setup ret: %d", ret);
		result = -2;
		goto do_return;
	}

do_return:
	return result;
}

void io_adc_static_thread(void* arg1, void* arg2, void* arg3)
{
	int ret = -1;

	(void)io_adc_init(g_adc_dev);

	int x = 0;
    while (1) {
		uint32_t value = 0;

		ret = adc_read(g_adc_dev, &sequence0);
		if (ret < 0) {
			LOG_ERR("adc_read 0 ret: %d", ret);
			goto do_return;
		}

		value = ((uint32_t)g_buf[0] + (uint32_t)g_buf[1] + (uint32_t)g_buf[2] + (uint32_t)g_buf[3] + 2) / 4;
		LOG_INF("adc ch0 value: %d", value);
		g_buf[0] = g_buf[1];
		g_buf[1] = g_buf[2];
		g_buf[2] = g_buf[3];

        k_usleep(250 * 1000);
    }

do_return:
	return;
}

#define STACKSIZE (512)
#define PRIORITY (10)
K_THREAD_DEFINE(io_adc, STACKSIZE, io_adc_static_thread, NULL, NULL, NULL,PRIORITY, 0, 0);