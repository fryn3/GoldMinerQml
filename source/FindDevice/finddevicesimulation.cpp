#include "finddevicesimulation.h"

FindDeviceSimulation::FindDeviceSimulation(QObject *parent)
    : FindDeviceControllerBase{parent} { }

void FindDeviceSimulation::start() {
    emit started();
    emit findedDevice("1.1.1.1", "AA:BB:CC:DD:EE:FF");
    if (++_counter % 2) {
        emit findedDevice("1.1.1.2", "AA:BB:CC:DD:EE:FF");
        emit findedDevice("1.1.1.3", "AA:BB:CC:DD:EE:FF");
    }
    emit finished();
}
