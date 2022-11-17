#pragma once

#include "finddevicecontrollerbase.h"

class FindDeviceExternalApp : public FindDeviceControllerBase {
    Q_OBJECT
public:
    explicit FindDeviceExternalApp(bool isDebugMode, QObject *parent = nullptr);

    QStringList firstSuitableNames { "lwip0" };

    const bool DEBUG_MODE = false;

    void start() override;
private:
    static int counterFindDev;
};

