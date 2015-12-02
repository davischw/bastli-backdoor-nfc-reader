# NFC reader part of the Bastli Backdoor

Daemon to run on the system connected to the NFC reader. This program
will connect to the main server and coordinate between the reading hardware
and the server.

## Installation (Reader and Opener)

The nfc-reader and nfc-opener are designed to be run on a Raspberry Pi.

### Dependencies

On Raspbian Jessie, the following packages have to be installed

    cmake libnfc-dev libfreefare-dev libboost-log-dev libboost-program-options-dev libboost-regex-dev librabbitmq-dev

The [SimpleAmqpClient library](https://github.com/alanxz/SimpleAmqpClient) has to be compiled and installed manually.
Follow the instructions on the library homepage and download, compile and install the library.
After the library is installed, make sure to run ldconfig, otherwise the program
will be unable to load the shared library:

    $ sudo ldconfig

### Download & Compilation

After installing all dependencies, the following steps install the actual
backdoor tools.

    $ git clone https://code.bastli.ch/dominik/backdoor-nfc-reader.git
    $ cd backdoor-nfc-reader
    $ cmake .
    $ make
    $ sudo make install
    

## NFC Reader configuration

To enable libnfc to find the NFC-Reader, place a config file at `/etc/nfc/devices.d/bastli_reader.conf`
with the following content, where `/dev/ttyUSB0` has to be the actual
USB serial device of the reader.

    name = "Bastli Reader"
    connstring = "pn532_uart:/dev/ttyUSB0"

## Autostart configuration

SystemD is used to automatically start the reader and opener on system boot
as service. To enable this, run the following commands, which create a backdoor
user, adds it to the dialout group (needed for access rights to usb serial ports),
and then enable automatic startup.

    $ sudo adduser backdoor --system --no-create-home --group
    $ sudo usermod -a -G dialout,gpio backdoor
    $ sudo systemctl enable backdoor-reader
    $ sudo systemctl enable backdoor-opener

To query the state of the background service:

    $ sudo systemctl status backdoor-reader
    $ sudo systemctl status backdoor-opener
    
To get the log output:

    $ sudo journalctl -u backdoor-reader
    $ sudo journalctl -u backdoor-opener