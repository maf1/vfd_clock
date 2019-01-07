# vfd_clock

This is an extended firmware for the Elektor VFD clock (https://www.elektormagazine.com/labs/6-digit-vfd-clock-with-esp32).
It is based on the version of 2018-10-05 by Peter S'heeren, Axiris, and is published under the same public domain licence.

The following features are added::
* Support for DST (EU time only, but easy to modify/extend, see top of clock.ino)
* Support for BME280, BMP280 sensors (new file bme280.ino)
* Display humidity and pressure (incl. 3h trend inidcator) if available
* Cycle through auxiliary data (date, temperature, humidity, pressure) when button is pushed and display is on
* Debouncing of button
* New commands to control above functions and read data (see comments in cmd_proc.ino)
* timer.ino to avoid complications of 32bit overflow of millis() (only used in new functions)
* New commands "help" and "system status"
* Show firmware revision at boot time

If a BM\*280 sensor is connected and enabled, its temperature readings will used and supersed other sensors.
BM\*280 sensors are easily available on breakout boards. The sensor is simply connected via the I2C interface K7 (take care not to confuse the signals as some boards have a different pinout than the connector K7). No further circuitry is necessary, as signal levels match and pull-ups are provided by the VFD clock board.

In addition to the Time library, this version needs the following modules (use the Arduino Library manager to load them if unavailable): 

- Timezone by Jack Christensen  
  Version 1.2.2  (Warning about architecture mismatch can be ignored)  
  https://github.com/JChristensen/Timezone

- BME280 by Tyler Glenn  
  Version 2.3.0  
  https://github.com/finitespace/BME280

Note that this software was not tested with all possible hardware and setting configurations and is in this sense to be considered experimental.
Only limited support can be given, you might use the firmware as is or for own experiments, in any case you do this totally on your own risk.
No liability for any direct or indirect damage will be accepted.

