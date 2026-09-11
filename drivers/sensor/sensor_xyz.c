#define DT_DRV_COMPAT sensor_xyz
#include "sensor_xyz.h"
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

static int sensor_xyz_sample_fetch(const struct device *dev,
				  enum sensor_channel chan)
{
	(void)dev;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_ACCEL_XYZ) {
		return -ENOTSUP;
	}

	return 0;
}

static int sensor_xyz_channel_get(const struct device *dev,
				 enum sensor_channel chan,
				 struct sensor_value *val)
{
	(void)dev;

	if (chan != SENSOR_CHAN_ACCEL_XYZ) {
		return -ENOTSUP;
	}

	val[0] = (struct sensor_value){ .val1 = 1, .val2 = 0 };
	val[1] = (struct sensor_value){ .val1 = 2, .val2 = 0 };
	val[2] = (struct sensor_value){ .val1 = 3, .val2 = 0 };

	return 0;
}

static DEVICE_API(sensor, sensor_xyz_api) = {
	.sample_fetch = sensor_xyz_sample_fetch,
	.channel_get = sensor_xyz_channel_get,
};

#define SENSOR_XYZ_DEFINE(inst)                                    \
	DEVICE_DT_INST_DEFINE(inst, NULL, NULL, NULL, NULL,         \
			      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, \
			      &sensor_xyz_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_XYZ_DEFINE)