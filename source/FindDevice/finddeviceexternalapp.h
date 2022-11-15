#pragma once

#include "finddevicecontrollerbase.h"

class FindDeviceExternalApp : public FindDeviceControllerBase {
    Q_OBJECT
public:
    explicit FindDeviceExternalApp(bool isDebugMode, QObject *parent = nullptr);

    QStringList firstSuitableNames { "lwip0" };
    QStringList debugSuitableNames { "androidName" }; /// \todo FIXME

    // В релизной версии нужно перевести в false!
    const bool DEBUG_MODE = true;

    void start() override;
private:
    int _counter = 0;
};

