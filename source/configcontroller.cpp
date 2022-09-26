#include "configcontroller.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ConfigController::ConfigController() noexcept {
    QFile f(CONFIG_FILE);
    if (!f.open(QIODevice::ReadOnly)) {
        setError(Error::CantOpenFile);
        return;
    }
    QJsonParseError jError;
    auto jDoc = QJsonDocument::fromJson(f.readAll(), &jError);
    if (jError.error != QJsonParseError::NoError) {
        setError(Error::BadJsonFormat);
        return;
    }
    if (jDoc.isEmpty()) {
        return;
    }
    if (!jDoc.isObject()) {
        setError(Error::BadJsonFormat);
        return;
    }
    const auto&& jObj = jDoc.object();
    {
        const auto&& jObjAuto = jObj.value(KEY_AUTO_DONLOADING_PATH);
        if (jObjAuto.isUndefined()) {
            return;
        }
        if (!jObjAuto.isString()) {
            setError(Error::BadJsonFormat);
            return;
        }
        autoDownloadingPath = jObjAuto.toString();
    }
    {
        const auto&& jObjParallel = jObj.value(KEY_COUNT_PARALLEL);
        if (jObjParallel.isUndefined()) {
            return;
        }
        if (!jObjParallel.isDouble()) {
            setError(Error::BadJsonFormat);
            return;
        }
        countParallel = jObjParallel.toDouble(DEFAULT_PARALLEL);
        if (countParallel < 1) {
            setError(Error::BadParallelValue);
            return;
        }
    }
    {
        const auto&& jObjDevices= jObj.value(KEY_DEVICES);
        if (jObjDevices.isUndefined()) {
            return;
        }
        if (!jObjDevices.isArray()) {
            setError(Error::BadJsonFormat);
            return;
        }
        const auto&& jArrayDevs = jObjDevices.toArray();
        for (const auto&& jDevValue: jArrayDevs) {
            if (!jDevValue.isObject()) {
                setError(Error::BadJsonFormat);
                return;
            }
            const auto&& jDevObj = jDevValue.toObject();
            if (!jDevObj.contains(KEY_IP)
                    || !jDevObj.contains(KEY_MAC)) {
                continue;
            }
            const auto&& jDevMacObj = jDevObj.value(KEY_MAC);
            const auto&& jDevIpObj = jDevObj.value(KEY_IP);

            if (!jDevMacObj.isString()
                    || !jDevIpObj.isString()) {
                setError(Error::BadJsonFormat);
                return;
            }
            devices.insert(jDevMacObj.toString(), jDevIpObj.toString());
        }
    }
}

ConfigController::~ConfigController() noexcept {
    QJsonObject jObj;
    jObj.insert(KEY_AUTO_DONLOADING_PATH, autoDownloadingPath);
    jObj.insert(KEY_COUNT_PARALLEL, countParallel);
    QJsonArray jArr;
    for (auto pairIt = devices.cbegin(); pairIt != devices.cend(); ++pairIt) {
        jArr.push_back(QJsonObject {
                           {KEY_MAC, pairIt.key()},
                           {KEY_IP, pairIt.value()}
                       });
    }
    jObj.insert(KEY_DEVICES, jArr);
    QFile f(CONFIG_FILE);
    if (!f.open(QIODevice::WriteOnly)) {
        // В деструкторе нет смысла выставлять ошибку!
        qDebug() << __FILE__ << __LINE__ << "can't open file";
        return;
    }
    QJsonDocument jDoc(jObj);
    f.write(jDoc.toJson());
}

ConfigController::Error ConfigController::error() const noexcept {
    return _error;
}

void ConfigController::setError(Error newError) noexcept {
    _error = newError;
    if (_error != Error::NoError) {
        autoDownloadingPath.clear();
        countParallel = DEFAULT_PARALLEL;
        devices.clear();
    }
}
