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

Note that this software was not tested with all possible hardware and setting configurations and is in this sense to be considered experimental.
Only limited support can be given, you might use the firmware as is or for own experiments, in any case you do this totally under your own risk.
No liability for any direct or indirect damage will be accepted.

