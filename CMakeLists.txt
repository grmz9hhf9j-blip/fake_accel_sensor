zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_SENSOR_XYZ
  drivers/sensor/sensor_xyz.c
)

zephyr_library_include_directories(include)