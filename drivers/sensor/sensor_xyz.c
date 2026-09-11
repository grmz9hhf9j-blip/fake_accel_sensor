#define DT_DRV_COMPAT sensor_xyz
#include "sensor_xyz.h"
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/sys/util.h>

#define SENSOR_XYZ_WAVE_PERIOD_MS 4000
#define SENSOR_XYZ_WAVE_HALF_PERIOD_MS 2000
#define SENSOR_XYZ_Y_OFFSET_MS 1000
#define SENSOR_XYZ_AMPLITUDE_MICRO 1000000
#define SENSOR_XYZ_MICRO_PER_UNIT 1000000

static const struct sensor_value sensor_xyz_gravity = {
	.val1 = 9,
	.val2 = 806650,
};
static struct sensor_value sensor_xyz_triangle(int32_t phase_ms)
{
	int32_t position = phase_ms % SENSOR_XYZ_WAVE_PERIOD_MS;
	int32_t micro;

	if (position > SENSOR_XYZ_WAVE_HALF_PERIOD_MS)
	{
		position = SENSOR_XYZ_WAVE_PERIOD_MS - position;
	}

	micro = -SENSOR_XYZ_AMPLITUDE_MICRO +
			(int32_t)((int64_t)position *
					  (2 * SENSOR_XYZ_AMPLITUDE_MICRO) /
					  SENSOR_XYZ_WAVE_HALF_PERIOD_MS);

	return (struct sensor_value){
		.val1 = micro / SENSOR_XYZ_MICRO_PER_UNIT,
		.val2 = micro % SENSOR_XYZ_MICRO_PER_UNIT,
	};
}

static struct sensor_xyz_sample sensor_xyz_generate(void)
{
	int32_t phase_ms = (int32_t)(k_uptime_get() % SENSOR_XYZ_WAVE_PERIOD_MS);

	return (struct sensor_xyz_sample){
		.ax_ms2 = sensor_xyz_triangle(phase_ms),
		.ay_ms2 = sensor_xyz_triangle(
			phase_ms + SENSOR_XYZ_Y_OFFSET_MS),
	};
}

static void sensor_xyz_timer_handler(struct k_timer *timer)
{
	struct sensor_xyz_data *data =
		CONTAINER_OF(timer, struct sensor_xyz_data, timer);
	struct sensor_xyz_sample sample = sensor_xyz_generate();
	k_spinlock_key_t key = k_spin_lock(&data->lock);

	data->latest = sample;

	k_spin_unlock(&data->lock, key);
}

static int sensor_xyz_sample_fetch(const struct device *dev,
								   enum sensor_channel chan)
{
	struct sensor_xyz_data *data = dev->data;
	k_spinlock_key_t key;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_ACCEL_XYZ)
	{
		return -ENOTSUP;
	}

	key = k_spin_lock(&data->lock);

	data->fetched = data->latest;
	data->fetched_valid = true;

	k_spin_unlock(&data->lock, key);

	return 0;
}

static int sensor_xyz_channel_get(const struct device *dev,
								  enum sensor_channel chan,
								  struct sensor_value *val)
{
	struct sensor_xyz_data *data = dev->data;
	struct sensor_xyz_sample sample;
	k_spinlock_key_t key;

	if (chan != SENSOR_CHAN_ACCEL_XYZ)
	{
		return -ENOTSUP;
	}

	key = k_spin_lock(&data->lock);

	if (!data->fetched_valid)
	{
		k_spin_unlock(&data->lock, key);
		return -EAGAIN;
	}

	sample = data->fetched;

	k_spin_unlock(&data->lock, key);

	val[0] = sample.ax_ms2;
	val[1] = sample.ay_ms2;
	val[2] = sensor_xyz_gravity;

	return 0;
}

static int sensor_xyz_init(const struct device *dev)
{
	const struct sensor_xyz_config *config = dev->config;
	struct sensor_xyz_data *data = dev->data;
	k_timeout_t period =
		K_USEC(DIV_ROUND_UP(1000000U, config->rate_hz));

	data->latest = sensor_xyz_generate();
	data->fetched_valid = false;

	k_timer_init(&data->timer, sensor_xyz_timer_handler, NULL);
	k_timer_start(&data->timer, period, period);

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
	static struct sensor_xyz_data sensor_xyz_data_##inst;              \
	DEVICE_DT_INST_DEFINE(inst, sensor_xyz_init, NULL,                 \
						  &sensor_xyz_data_##inst,                     \
						  &sensor_xyz_config_##inst,                   \
						  POST_KERNEL,                                 \
						  CONFIG_SENSOR_INIT_PRIORITY,                 \
						  &sensor_xyz_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_XYZ_DEFINE)