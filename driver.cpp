#include "config.h"
#include "driver.h"
#include <errno.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

static std::unique_ptr<SerialShutterReleaseCCD> driver(new SerialShutterReleaseCCD());
constexpr int RTS = TIOCM_RTS;
constexpr char DEVICE_NAME[MAXINDIDEVICE] = "Serial Shutter Release";

SerialShutterReleaseCCD::SerialShutterReleaseCCD() : fd(-1) {
    setVersion(INDI_SERIAL_SHUTTER_RELEASE_VERSION_MAJOR, INDI_SERIAL_SHUTTER_RELEASE_VERSION_MINOR);
}

const char* SerialShutterReleaseCCD::getDefaultName() {
    return DEVICE_NAME;
}

bool SerialShutterReleaseCCD::initProperties() {
    setDeviceName(getDefaultName());
    CCD::initProperties();

    IUFillText(&mPortT[0], "SERIAL_PORT", "Serial Port", "");
    IUFillTextVector(&PortTP, mPortT, NARRAY(mPortT), getDeviceName(), "DEVICE_PORT", "Shutter Release", CONNECTION_TAB, IP_RW, 0, IPS_IDLE);

    addAuxControls(); // debug, simulator, configuration, and poll period controls

    SetCCDCapability(CCD_HAS_SHUTTER | CCD_CAN_ABORT);
    SetCCDParams(6026, 4017, 8, 3.9, 3.9); // sensor info for my Fujifilm X-A5
    PrimaryCCD.setMinMaxStep("CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", 0.001, 3600, 1, false); // allow exposures ranging from 1ms to 1hr, with default steps of 1 second
    PrimaryCCD.getCCDInfo()->p = IP_RW; // i think this allows end users to set ccd info to whatever they like, but tbh i'm just copying the lead of the gphoto driver here
    PrimaryCCD.setFrameBufferSize(0);

    // upload mode should always be local, no images uploaded
    if (UploadS[UPLOAD_LOCAL].s != ISS_ON)
    {
        IUResetSwitch(&UploadSP);
        UploadS[UPLOAD_LOCAL].s = ISS_ON;
        UploadSP.s = IPS_OK;
        IDSetSwitch(&UploadSP, nullptr);
    }

    return true;
}

bool SerialShutterReleaseCCD::updateProperties() {
    CCD::updateProperties();
    // driver doesn't have any properties other than the serial port, which is always active, therefore no updates are ever needed
    return true;
}

bool SerialShutterReleaseCCD::saveConfigItems(FILE * fp) {
    CCD::saveConfigItems(fp);

    if (PortTP.tp[0].text)
        IUSaveConfigText(fp, &PortTP);
    IUSaveConfigNumber(fp, PrimaryCCD.getCCDInfo());

    return true;
}

bool SerialShutterReleaseCCD::ISNewText(const char *dev, const char *name, char** texts, char *names[], int n) {
    if (dev != nullptr && 0 == strcmp(dev, getDeviceName())) {
        if (0 == strcmp(name, PortTP.name)) {
            const char* prev = mPortT[0].text;
            IUUpdateText(&PortTP, texts, names, n);

            if (isConnected() && (!prev || strcmp(prev, mPortT[0].text)))
            {
                PortTP.s = IPS_BUSY;
                LOG_INFO("Please restart the driver for this change to have effect.");
            } else {
                PortTP.s = IPS_OK;
            }
            saveConfig(true, PortTP.name);
            IDSetText(&PortTP, nullptr);

            return true;
        }
    }

    // fallthroughs are processed by the parent class
    return DefaultDevice::ISNewText(dev, name, texts, names, n);
}

void SerialShutterReleaseCCD::ISGetProperties(const char * dev) {
    CCD::ISGetProperties(dev);

    /* char port_trunc[MAXINDINAME] = {0};
    // ? why does the gphoto driver seemingly go out of its way to truncate its own text
    if (IUGetConfigText(getDeviceName(), PortTP.name, mPortT[0].name, port_trunc, MAXINDINAME) == 0 && port_trunc[0] != '\0')
        IUSaveText(&mPortT[0], port_trunc); */
    defineProperty(&PortTP);
}

bool SerialShutterReleaseCCD::Connect() {
    if (PortTP.tp[0].text && strlen(PortTP.tp[0].text) > 0) {
        // read-write capabilities shouldn't matter since i'm not actually doing any reading or writing, just flipping the RTS flag back and forth
        // however, O_NOCTTY is really important, as it prevents creation of a virtual terminal that thinks it's smart enough to handle this flag for us
        fd = open(PortTP.tp[0].text, O_RDWR | O_NOCTTY);
        if (fd != -1) {
            return true;
        } else {
            LOG_ERROR(fmt::format("Failed to connect to shutter release port {}: {}", PortTP.tp[0].text, strerror(errno)).c_str());
        }

        termios tty;
        if (tcgetattr(fd, &tty) == -1) {
            LOG_ERROR(fmt::format("Failed to get tty attributes for shutter release connection: {}", strerror(errno)).c_str());
            return false;
        }
        tty.c_cflag &= CRTSCTS; // enable manual control over RTS/CTS flags
        if (tcsetattr(fd, TCSANOW, &tty) == -1) {
            LOG_ERROR(fmt::format("Failed to set tty attributes for shutter release connection: {}", strerror(errno)).c_str());
            return false;
        }
        if (ioctl(fd, TIOCMBIC, &RTS) == -1) {
            LOG_ERROR(fmt::format("Failed to clear RTS bit at start of shutter release connection: {}", strerror(errno)).c_str());
            return false;
        }
        ioctl(fd, TIOCMBIC, &RTS); // no guarantees that RTS starts disabled
    } else {
        LOG_ERROR("Failed to connect: need to specify a shutter release port");
    }
    PortTP.s = IPS_ALERT;
    IDSetText(&PortTP, nullptr);
    return false;
}

bool SerialShutterReleaseCCD::Disconnect() {
    if (InExposure)
        AbortExposure();
    close(fd);
    fd = -1;
    return true;
}

bool SerialShutterReleaseCCD::StartExposure(float duration) {
    if (!InExposure) {
        // set the RTS flag on the device
        if (ioctl(fd, TIOCMBIS, &RTS) == -1) {
            LOG_ERROR(fmt::format("Failed to start exposure via serial connection: {}", strerror(errno)).c_str());
            return false;
        }

        PrimaryCCD.setExposureDuration(duration);
        InExposure = true;
        exposure_req = duration;
        gettimeofday(&exposure_start, nullptr);
        if (duration < getCurrentPollingPeriod() / 1000.0)
            exposure_timer_id = SetTimer(static_cast<uint32_t>(duration * 950)); // 95% of the requested exposure length
        else
            exposure_timer_id = SetTimer(getCurrentPollingPeriod());

        LOG_DEBUG(fmt::format("Started {:.3f}s exposure", duration).c_str());
        return true;
    } else {
        LOG_ERROR("Cannot start exposure while another is already in progress");
        return false;
    }
}

bool SerialShutterReleaseCCD::AbortExposure() {
    if (InExposure) {
        RemoveTimer(exposure_timer_id);
        InExposure = false;
        PrimaryCCD.setExposureFailed(); // ? idk whether to use this or setExposureComplete, but am leaning towards this

        if (ioctl(fd, TIOCMBIC, &RTS) == -1) {
            LOG_ERROR(fmt::format("Failed to abort exposure via serial connection: {}", strerror(errno)).c_str());
            return false;
        } else {
            LOG_DEBUG(fmt::format("Aborted {:.3f}s exposure", exposure_req).c_str());
            return true;
        }
    } else {
        LOG_ERROR("Failed to abort exposure: no exposure is active!");
        return false;
    }
}

double SerialShutterReleaseCCD::ExposureTimeLeft() {
    struct timeval now, diff;
    gettimeofday(&now, nullptr);

    timersub(&now, &exposure_start, &diff);
    double timesince = diff.tv_sec + diff.tv_usec / 1e6;
    return (exposure_req - timesince);
}

void SerialShutterReleaseCCD::TimerHit() {
    if (isConnected() && InExposure) {
        double timeleft = std::max(ExposureTimeLeft(), 0.0);
        if (timeleft < 0.001) { // aim for millisecond-ish accuracy
            PrimaryCCD.setExposureLeft(0);
            InExposure = false;

            if (ioctl(fd, TIOCMBIC, &RTS) == -1) {
                LOG_ERROR(fmt::format("Failed to finish exposure via serial connection: {}", strerror(errno)).c_str());
            } else {
                LOG_DEBUG(fmt::format("Finished {:.3f}s exposure", exposure_req).c_str());
            }
            // PrimaryCCD.setFrameBufferSize(0); // ? do i have to reset this every exposure or am i the only thing capable of changing it
            ExposureComplete(&PrimaryCCD);
        } else {
            // informed by https://github.com/indilib/indi/blob/4f9db311fa6305b32cd3bcb1fe7cae904e79c7fc/libs/indibase/indiccd.cpp#L2510
            if (timeleft * 1000 < getCurrentPollingPeriod())
                setCurrentPollingPeriod(timeleft * 950);

            PrimaryCCD.setExposureLeft(timeleft);
            exposure_timer_id = SetTimer(getCurrentPollingPeriod());
        }
    }
}

bool SerialShutterReleaseCCD::UpdateCCDUploadMode(CCD_UPLOAD_MODE mode) {
    return (mode == UPLOAD_LOCAL);
}
