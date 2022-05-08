
#pragma once

#include <indiccd.h>
#include <string>
#include <semaphore.h>

class SerialShutterReleaseCCD : public INDI::CCD
{
public:
    SerialShutterReleaseCCD();
    virtual ~SerialShutterReleaseCCD() = default;

    virtual const char* getDefaultName() override;

    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    bool saveConfigItems(FILE* fp) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n);
    virtual void ISGetProperties(const char * dev) override;

    bool Connect() override;
    bool Disconnect() override;

    bool StartExposure(float duration) override;
    bool AbortExposure() override;
    void TimerHit() override;
    double ExposureTimeLeft(); // in seconds
    bool UpdateCCDUploadMode(CCD_UPLOAD_MODE mode) override;
protected:
    IText mPortT[1] {};
    ITextVectorProperty PortTP;
    int fd;

    timeval exposure_start;
    double exposure_req;
    int exposure_timer_id;
};
