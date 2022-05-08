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

# Using this Driver in KStars/Ekos

Open KStars, then open Ekos by clicking the icon fifth from the right in the top menu, which looks like an open observatory. If this is your first time using Ekos, you'll need to add a new profile using the leftmost of the small buttons in the section labeled "1. Select Profile", which has a plus sign icon. If you've used Ekos before, you may prefer to edit an existing profile with the pencil-icon button just to the right. These should both open the Profile Editor in a popup window. Expand the combo box labelled "CCD" at the top left of "Select Devices" section. **This driver is the entry "Serial Shutter Release" under the category "DSLRs".** At minimum, a profile needs a mount and a CCD specified -- but one or both can be spoofed with simulators by selecting the appropriate entry in the corresponding combo box, which have their own "Simulators" category. Configure the remainder of the profile (name, other devices, guiding software and associated telescopes) to your liking, then save and exit the Profile Editor.

Return to the Ekos window and click the play button in the "2. Start & Stop Ekos" section. This will open the INDI Control Panel in a popup window. Navigate to the "Serial Shutter Release" tab, then the "Connection" subtab. Enter the path to the serial port you wish to use (usually something like `/dev/ttyUSB0` or `/dev/serial/by-id/...`), and click the "Set" button. Go back to the "Main Control" subtab and click "Connect". You are now controlling your camera's shutter with INDI! In order to schedule a series of exposures, head over to the CCD tab that has appeared in the main Ekos window, marked with an orange circle with a camera icon. Set Exposure, Count, and Delay to your liking, click the plus button in the top left of the "Sequence Queue section", and then the play button in the bottom right to begin the exposures.
