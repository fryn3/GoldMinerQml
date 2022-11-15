#pragma once

#include "finddevicecontrollerbase.h"

class FindDeviceSimulation : public FindDeviceControllerBase {
    Q_OBJECT
public:
    explicit FindDeviceSimulation(QObject *parent = nullptr);

    void start() override;
private:
    int _counter = 0;
};

