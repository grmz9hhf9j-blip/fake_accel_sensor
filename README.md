## Sensor driver

The driver implements `sample_fetch` and `channel_get` for
`SENSOR_CHAN_ACCEL_XYZ`.

A timer generates X/Y triangle waves with a four-second period and an
amplitude of 1 m/s². Y is shifted by one second. Z remains at
9.80665 m/s².

Fetch copies the latest generated sample into a snapshot. Get returns
that snapshot. A spinlock protects shared sample copies.

The module registers the custom `sensor` vendor prefix so the required
`sensor,xyz` compatible passes Devicetree validation.


