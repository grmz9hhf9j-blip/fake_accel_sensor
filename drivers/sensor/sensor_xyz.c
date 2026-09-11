#define DT_DRV_COMPAT sensor_xyz
#include "sensor_xyz.h"
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/sys/util.h>

static const struct sensor_value sensor_xyz_gravity = {
	.val1 = 9,
	.val2 = 806650,
};

static int sensor_xyz_sample_fetch(const struct device *dev,
								   enum sensor_channel chan)
{
	struct sensor_xyz_data *data = dev->data;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_ACCEL_XYZ)
	{
		return -ENOTSUP;
	}

	data->fetched = data->latest;
	data->fetched_valid = true;

	return 0;
}

static int sensor_xyz_channel_get(const struct device *dev,
								  enum sensor_channel chan,
								  struct sensor_value *val)
{
	const struct sensor_xyz_data *data = dev->data;

	if (chan != SENSOR_CHAN_ACCEL_XYZ)
	{
		return -ENOTSUP;
	}

	if (!data->fetched_valid)
	{
		return -EAGAIN;
	}

	val[0] = data->fetched.ax_ms2;
	val[1] = data->fetched.ay_ms2;
	val[2] = sensor_xyz_gravity;

	return 0;
}

static DEVICE_API(sensor, sensor_xyz_api) = {
	.sample_fetch = sensor_xyz_sample_fetch,
	.channel_get = sensor_xyz_channel_get,
};

#define SENSOR_XYZ_RATE(inst) \
	DT_INST_PROP_OR(inst, odr_hz, CONFIG_SENSOR_XYZ_RATE_HZ)

#define SENSOR_XYZ_DEFINE(inst)                                        \
	BUILD_ASSERT(SENSOR_XYZ_RATE(inst) >= 1 &&                         \
					 SENSOR_XYZ_RATE(inst) <= 1600,                    \
				 "sensor,xyz odr-hz must be between 1 and 1600");      \
	static const struct sensor_xyz_config sensor_xyz_config_##inst = { \
		.rate_hz = SENSOR_XYZ_RATE(inst),                              \
	};                                                                 \
	static struct sensor_xyz_data sensor_xyz_data_##inst = {           \
		.latest = {                                                    \
			.ax_ms2 = {.val1 = 1, .val2 = 0},                          \
			.ay_ms2 = {.val1 = 2, .val2 = 0},                          \
		},                                                             \
		.fetched_valid = false,                                        \
	};                                                                 \
	DEVICE_DT_INST_DEFINE(inst, NULL, NULL,                            \
						  &sensor_xyz_data_##inst,                     \
						  &sensor_xyz_config_##inst,                   \
						  POST_KERNEL,                                 \
						  CONFIG_SENSOR_INIT_PRIORITY,                 \
						  &sensor_xyz_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_XYZ_DEFINE)