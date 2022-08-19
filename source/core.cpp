#include "core.h"
#include "myfunc.h"
#include "qcoreapplication.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>

const QString Core::ITEM_NAME = "Core";
static void regT() {
    My::qmlRegisterType<Core>(Core::ITEM_NAME);
}
Q_COREAPP_STARTUP_FUNCTION(regT)
const bool Core::IS_QML_REG = true;//My::qmlRegisterType<Core>(Core::ITEM_NAME);

Core::Core(QObject *parent)
    : QObject{parent} {

}

void Core::findDevices() {
    setState(State::FindingDevices);
    _devModel.clear();
    QFile findIpExe(":/resources/soft/FindIP.exe");
    QTemporaryFile *newTempFile = QTemporaryFile::createNativeFile(findIpExe);
    newTempFile->rename(newTempFile->fileName() + ".exe");
    QProcess *p = new QProcess(this);
    connect(p, &QProcess::errorOccurred, this, [this, newTempFile] {
        emit showMessage(QString("Ошибка запуска %1")
                                   .arg(newTempFile->fileName()), -1);
        setState(State::None);
        sender()->deleteLater();
        newTempFile->remove();
        newTempFile->deleteLater();
    });

    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            p, &QProcess::deleteLater);
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, newTempFile] {
        emit showMessage("Поиск устройств завершен", 5000);
        setState(State::None);
        newTempFile->remove();
        newTempFile->deleteLater();
    });
    connect(p, &QProcess::readyReadStandardOutput, this, [this] {
        auto p = dynamic_cast<QProcess*>(sender());
        QString s = p->readAllStandardOutput();
        auto rows = s.split("\r\n", Qt::SkipEmptyParts);
        for (auto &row: rows) {
            auto words = row.split(" ");
            auto ip = words.at(1);
            auto mac = words.at(3);
            auto oName = words.at(5);
            if (oName != "lwip0") {
                continue;
            }
            _devModel.addDevice(ip, mac, oName);
        }
    });
    p->start(newTempFile->fileName(), {"VideoServer"});
    emit showMessage("Поиск устройств...");
}

Core::State Core::state() const {
    return _state;
}

void Core::setState(State newState) {
    if (_state == newState)
        return;
    _state = newState;
    emit stateChanged();
}

DeviceModel * const Core::devModel() {
    return &_devModel;
}
