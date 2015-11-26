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
    
    