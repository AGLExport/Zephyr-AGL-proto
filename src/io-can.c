#include "io-can.h"
#include <zephyr/logging/log.h>
#include <zephyr/drivers/can.h>

//-------------------------------------------------------------
LOG_MODULE_REGISTER(io_can);

//-------------------------------------------------------------
// Global
// CAN device
const struct device *const g_can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

// CAN receive ids
typedef struct s_io_can_receive_id {
	uint32_t id;
} io_can_receive_id_t;

const io_can_receive_id_t receive_ids[] = {
	{ .id = 0x3C3, },
	{ .id = 0x400, },
	{ .id = 0x000, },
	{ .id = 0x333, },
};
//-------------------------------------------------------------
static int io_can_timing_init(const struct device *const can_dev)
{
	int ret = 0;
	int result = 0;
	struct can_timing timing = {0};

	if (can_dev == NULL) {
		result = -1;
		goto do_return;
	}

	// Set Bitrate:500kbps, Sample point: 87.5%
	ret = can_calc_timing(can_dev, &timing, 500000, 875);
	if (ret != 0) {
		result = -2;
		LOG_ERR("CAN setting does not support Bitrate:500kbps, Sample point: 87.5");
		goto do_return;
	}

	ret = can_set_timing(can_dev, &timing);
	if (ret != 0) {
		result = -3;
		LOG_ERR("Error at CAN timing set to device");
		goto do_return;
	}

do_return:
	return result;
}
//-------------------------------------------------------------
static void can_rx_callback_proc(const struct device *dev, struct can_frame *frame, void *user_data)
{
    LOG_INF("ID:%X  dlc:%d  data: %x:%x:%x:%x:%x:%x:%x:%x\n"
            ,frame->id, frame->dlc
            ,frame->data[0],frame->data[1],frame->data[2],frame->data[3]
            ,frame->data[4],frame->data[5],frame->data[6],frame->data[7]);
}
//-------------------------------------------------------------
static int io_can_receive_init(const struct device *const can_dev)
{
	int ret = 0;
	int result = 0;
    struct can_filter filter = {
        .flags = 0,
        .id = 0,
        .mask = CAN_STD_ID_MASK,
    };

	if (can_dev == NULL) {
		result = -1;
		goto do_return;
	}

	// Set receive CAN frames.
	for (int i=0; i < (sizeof(receive_ids)/sizeof(io_can_receive_id_t)); i++) {
		filter.id = receive_ids[i].id;
		ret = can_add_rx_filter(can_dev, can_rx_callback_proc, NULL, &filter);
		if (ret < 0) {
			result = -2;
			LOG_ERR("Error at CAN receive setting to device");
			goto do_return;
		} else {
			LOG_INF("CAN id:%x set receive id: %d",filter.id, ret);
		}
	}

do_return:
	return result;
}

//-------------------------------------------------------------
static int io_can_init(const struct device *const can_dev)
{
	int ret = -1;
	int result = 0;

	if (!device_is_ready(can_dev)) {
		result = -1;
		goto do_return;
	}

	ret = io_can_timing_init(can_dev);
	if (ret != 0) {
		result = -2;
		goto do_return;
	}

	ret = io_can_receive_init(can_dev);
	if (ret != 0) {
		result = -3;
		goto do_return;
	}

   ret = can_start (can_dev);
	if (ret != 0) {
		result = -4;
		goto do_return;
	}

do_return:
	return result;
}

void io_can_static_thread(void* arg1, void* arg2, void* arg3)
{
    struct can_frame test_frame = {
        .flags = 0,
        .id = 0x123,
        .dlc = 8,
        .data = {1,2,3,4,5,6,7,8}
    };

	(void)io_can_init(g_can_dev);

	int x = 0;
    while (1) {
        (void)can_send(g_can_dev, &test_frame, K_MSEC(100), NULL, NULL);

        test_frame.id = (x % 4) * 0x123;
        x++;

        k_usleep(100 * 1000);
    }
}

#define STACKSIZE (512)
#define PRIORITY (10)
K_THREAD_DEFINE(io_can, STACKSIZE, io_can_static_thread, NULL, NULL, NULL,PRIORITY, 0, 0);
