#ifndef SENSOR_XYZ_DRIVER_H
#define SENSOR_XYZ_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

struct sensor_xyz_config {
    uint32_t rate_hz;
}

struct sensor_xyz_sample {
	struct sensor_value ax_ms2;
	struct sensor_value ay_ms2;
};

struct sensor_xyz_data {
	struct sensor_xyz_sample latest;
	struct sensor_xyz_sample fetched;
	bool fetched_valid;
};

#endif // SENSOR_XYZ_DRIVER_H