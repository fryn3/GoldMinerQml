#include "finddevicecontrollerbase.h"
#include "finddeviceexternalapp.h"
#include "finddevicesimulation.h"

FindDeviceControllerBase *FindDeviceControllerBase::createController(QObject *parent) {
    // В коде раскомментировать одну из трех строчек

//    /// Полная симуляция поиска девайсов
//    return new FindDeviceSimulation(parent);
    /// Поиск девайса с дебажными FTP серверами
    return new FindDeviceExternalApp(true, parent);
//    /// Релизный поиск девайса. В бою расскоментить эту строку!
//    return new FindDeviceExternalApp(false, parent);
}

FindDeviceControllerBase::FindDeviceControllerBase(QObject *parent)
    : QObject{parent} {
}

bool FindDeviceControllerBase::isError() {
    return _error;
}

const QString &FindDeviceControllerBase::errorMsg() const {
    return _errorMsg;
}

void FindDeviceControllerBase::setError(const QString &newErrorMsg) {
    _error = true;
    _errorMsg = newErrorMsg;
    emit error();
}
