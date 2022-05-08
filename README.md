# Serial Shutter Release INDI Driver

[INDI](https://indilib.org/) is an open-source system for distributed control of astronomy equipment. This is an INDI driver intended for amateur astrophotographers with DSLRs that do not offer the kind of tethering support required by existing DSLR drivers. It uses the RTS flag of an RS-232 connection to ultimately control the shutter release of a single camera. As the shutter release is the only point of communication, and it is one-way, no images can be saved to disk or live-viewed. The [indi-gphoto](https://github.com/indilib/indi-3rdparty/tree/master/indi-gphoto) driver also offers support for shutter control via serial port, but this is only available if gPhoto can connect to the camera itself with a protocol such as PTP, and is not available as a standalone feature for cameras which cannot tether using gPhoto or at all.

# Disclaimers

**This driver requires additional electronics in order to interface with the DSLR it will ultimately be controlling!** Nothing about toggling the RTS flag of a serial connection inherently has any relationship to camera shutters; it's just a commonly-used method. In order for this driver to control a camera shutter, this driver relies on hardware, mostly DIY, which can turn this signal into one that a camera can recognize -- such as CloudyNights user Poynting's [Fujifilm-Astro-USB-Controller](https://github.com/jconenna/Fujifilm-Astro-USB-Controller), which converts it to the [3-pole audio jack](https://www.doc-diy.net/photo/remote_pinout/#canon) method found in many modern cameras.

The way I've programmed this uses a feature which is not included in the POSIX standard (search for CRTSCTS in [this man page](https://linux.die.net/man/3/tcsetattr)), but my understanding is that it's common enough that this driver should work on an average Linux system. I may come back and search for an alternate solution if this presents problems, but make no promises. The software is provided as-is, etc etc.

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
