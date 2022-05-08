# Serial Shutter Release INDI Driver

This driver uses the RTS flag of an RS-232 connection to control the shutter release of a single camera. As the shutter release is the only point of communication, and it is one-way, no image output will be available. The [indi-gphoto](https://github.com/indilib/indi-3rdparty/tree/master/indi-gphoto) driver also offers support for shutter control via serial port, but this is only available if gPhoto can connect to the camera itself with a protocol such as PTP, and is not available as a standalone feature for cameras which cannot tether using gPhoto or at all. Written for use with (and exclusively tested with) CloudyNights user Poynting's [Fujifilm-Astro-USB-Controller](https://github.com/jconenna/Fujifilm-Astro-USB-Controller), but should work with any combination of camera and serial port shutter release system driven by the same mechanism.

# Building

Requires the [fmt](https://fmt.dev/latest/index.html) string formatting library, which is likely available from your system's package repositories. On Arch, the package is `fmt` from the `extra` repository. On Ubuntu, the package is `libfmt-dev` from the `universe` repository:

```
sudo add-apt-repository universe
sudo apt update
sudo apt install libfmt-dev
```

The source can be downloaded, compiled, and installed with the following commands:

```
git clone https://github.com/wrenby/serial-shutter-release
cd indi-serial-shutter-release
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ../
make
sudo make install
```

# Disclaimer

The way I've programmed this uses a feature which are not included in the POSIX standard (search for CRTSCTS in [this man page](https://linux.die.net/man/3/tcsetattr)), but my understanding is that it's common enough that this driver should work on an average Linux system. I may come back and search for an alternate solution if this presents problems, but make no promises. The software is provided as-is, etc etc.
