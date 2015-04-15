# NFC reader part of the Bastli Backdoor

Daemon to run on the system connected to the NFC reader. This program
will connect to the main server and coordinate between the reading hardware
and the server.

## Installation

### Dependencies

- libnfc
- libfreefare
- all the boost

### Download & Compilation

- Download git repo
- cd into git repo
- cmake .
- make

# Raspberry Pi Preparation

Install ARCH Linux, because Raspian fails

## I2C Configuration

Install the i2c-tools, mainly for debugging. 
Enable I2C in /boot/config.txt (see [Raspberry Pi Forum](https://www.raspberrypi.org/forums/viewtopic.php?p=675658#p675658)
Load the proper modules, namely i2c-dev and i2c_bcm2708, by adding them to /etc/modules-load.d/raspberrypi.conf.

Allow non-root access to i2c: [StackOverflow explanation](http://raspberrypi.stackexchange.com/questions/4468/i2c-group-on-arch)

