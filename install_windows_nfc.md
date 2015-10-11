# Building on Windows

## Prerequisites

The following tools and libraries have to be installed to compile
the Bastli Backdoor(TM) on Windows:

 - mingw64
 - boost
 - libnfc
 - libfreefare
 - rabbitmq-c

Make sure to install mingw64 first, as it is needed to compile the other
libraries.

### mingw64

Mingw64 can be downloaded by using the win-builds package manager,
available from http://win-builds.org/ .

On the first screen of the package manager, select `Native Windows` as the
system to install for and `i686` as the architecture. Choose an installation
directory, the instructions will refer to this directory as `WINBUILDS_INSTALL_DIR`.

On the next screen, the easiest way is to just install all packages, as there
is no dependency management, and it is unknown which packages are needed
for mingw....

Now, add `WINBUILDS_INSTALL_DIR/bin` to your $PATH.

### boost

Download the boost source code from http://www.boost.org/, and
extract it to a directory of your choice, in the future called `BOOST_SRC_DIR`.
Open a shell in `BOOST_SRC_DIR/tools/build` and run the following commands:

    ./bootstrap.bat mingw
    ./b2 toolset=gcc install --prefix=BOOST_BUILD_DIR

After that, add `BOOST_BUILD_DIR/bin` to your path. Then, reopen your
shell to make sure that the new path is working. Now, go back to the
boost source dir `BOOST_SRC_DIR`. Run the following command to compile
boost:

    b2 toolset=gcc address-model=32 -j 5 --with-system --with-filesystem --with-date_time --with-thread --with-log --with-program_options --with-regex stage

### cmake

Download the cmake Windows installer from the official cmake
website: https://cmake.org/
Install it, and make sure that you check the option to add it to your PATH.


### PCRE

Download pcre from here: http://gnuwin32.sourceforge.net/packages/pcre.htm,
use complete package, without source, and then select pcre again, then
version 7.0, and finally download pcre-7.0.exe and install PCRE.

### libnfc

Run the following commands in a shell with git to download the last libnfc version.

    git clone https://github.com/nfc-tools/libnfc.git
    cd libnfc
    git checkout libnfc-1.7.1
    cd ..
    mkdir libnfc_build
    cd libnfc_build

Now, open a new shell at the same location and run the following command to
open the cmake gui:

    cmake-gui.exe .

Select the libnfc folder as the source folder, and the libnfc_build folder
as the build folder. Then, click the Configure button and generate Mingw makefiles.
Then, you have to set the following variables:

PCRE_BIN_DIRS C:/Program Files (x86)/GnuWin32/bin
PCRE_INCLUDE_DIRS C:/Program Files (x86)/GnuWin32/include
PCRE_LIBRARIES C:/Program Files (x86)/GnuWin32/lib/pcre.lib

and make sure only the LIBNFC_DRIVER_PN532_UART driver is selected,
all the other drivers are not needed and can lead to compile errors.
Finally, click generate to actually generate the proper makefiles and run make:

    ming232-make

In an administrative shell, run:

    mingw32-make install

After this, libnfc is installed.

### libfreefare

Run the following commands in a shell with git to download the
last libfreefare version.

    git clone https://github.com/nfc-tools/libfreefare.git
    cd libfreefare
    git checkout libfreefare-0.4.0
    cd ..
    mkdir libfreefare_build
    cd libfreefare_build

Now, open a new shell (without git or sh.exe) at the same location and run the following command to
open the cmake gui:

    cmake-gui.exe .

Select the libnfc folder as the source folder, and the libnfc_build folder
as the build folder. Then, click the Configure button and generate Mingw makefiles.
Depending on your system, you might have to set the following variable, depending
on your Win-build installation directory:

WINSOCK_LIB WINBUILDS_INSTALL_DIR/mingw/lib/libws2_32.a

Afterwards, click genrate, then run make:

    mingw32-make

Then, with an administrative shell, run:

    mingw32-make install

After this, libfreefare should be installed.

### rabbitmq-c

Clone the git repo (https://github.com/alanxz/rabbitmq-c.git) and install
it using cmake. Make sure not to use the latest release (0.7.0), but the
git repo directly, as version 0.7.0 contains a bug which makes it unusable on
Windows.

## bastli backdoor (tm)

Now, you can finally begin to build the Bastli backdoor. Run cmake, set
the necessary variables to the libraries you installed, and it should (tm) work.
