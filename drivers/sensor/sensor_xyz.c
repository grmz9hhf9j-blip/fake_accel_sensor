#define DT_DRV_COMPAT sensor_xyz
#include "sensor_xyz.h"
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

static const struct sensor_value sensor_xyz_gravity = {
	.val1 = 9,
	.val2 = 806650,
};

static int sensor_xyz_sample_fetch(const struct device *dev,
								   enum sensor_channel chan)
{
	(void)dev;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_ACCEL_XYZ)
	{
		return -ENOTSUP;
	}

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

	val[0] = data->ax_ms2;
	val[1] = data->ay_ms2;
	val[2] = sensor_xyz_gravity;

	return 0;
}

static DEVICE_API(sensor, sensor_xyz_api) = {
	.sample_fetch = sensor_xyz_sample_fetch,
	.channel_get = sensor_xyz_channel_get,
};

#define SENSOR_XYZ_DEFINE(inst)                                      \
	static struct sensor_xyz_data sensor_xyz_data_##inst = {     \
		.ax_ms2 = { .val1 = 1, .val2 = 0 },                  \
		.ay_ms2 = { .val1 = 2, .val2 = 0 },                  \
	};                                                          \
	DEVICE_DT_INST_DEFINE(inst, NULL, NULL,                      \
			      &sensor_xyz_data_##inst, NULL,        \
			      POST_KERNEL,                          \
			      CONFIG_SENSOR_INIT_PRIORITY,          \
			      &sensor_xyz_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_XYZ_DEFINE)