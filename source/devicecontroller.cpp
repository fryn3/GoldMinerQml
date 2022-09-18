#include "devicecontroller.h"

#include "myfunc.h"

#include <QThread>

DeviceController::DeviceController(QObject *parent)
    : QObject{parent} { }

void DeviceController::setDeviceModel(DeviceModel *devModel) {
    _devModel = devModel;
}

void DeviceController::startDownload() {
    Q_ASSERT(_devModel);
    Q_ASSERT(_devsCommander.isEmpty());
    _currentDev = 0;

    for (; _currentDev < _countParallel; ++_currentDev) {
        if (_currentDev >= _devModel->rowCount()) {
            qDebug() << "В моделе девайсы закончились.";
            return;
        }

        auto dStruct = _devModel->get(_currentDev,
                                      DeviceModel::DmStructRole).value<DeviceCam>();
        _devsCommander.append(DevWorker{dStruct});

    }
}

void DeviceController::initFtpServer(int indexDev) {
    Q_ASSERT(!_devsCommander.contains(indexDev));
    QString username = My::genRandom(8);
    QString password = My::genRandom(8);

    _devModel->set(indexDev, username, DeviceModel::DmFtpUsernameRole);
    _devModel->set(indexDev, password, DeviceModel::DmFtpPasswordRole);

    auto dStruct = _devModel->get(_currentDev,
                                  DeviceModel::DmStructRole).value<DeviceCam>();

    _devsCommander.insert(indexDev, DevWorker(dStruct));


}

DevWorker::DevWorker(QString ipStr, QString ftpLog, QString ftpPass, QObject *parent)
    : QObject(parent), ip(ipStr), ftpUsername(ftpLog), ftpPassword(ftpPass) {
    connect(&_commander, &DeviceCommander::waitForAnswerChanged, this, &DevWorker::stateMachine);
    _commander.setIp(ip);
    _commander.setFtpUsername(ftpUsername);
    _commander.setFtpPassword(ftpPassword);
    _commander.setVideoRecordMode(mode);
}

DevWorker::DevWorker(const DeviceCam &dev, QObject *parent)
    : DevWorker(dev.ip, dev.ftpUsername, dev.ftpPassword, parent) { }

DevWorker::~DevWorker() {
    if (_commander) {
        delete _commander;
    }
    if (_ftpModel) {
        delete _ftpModel;
    }
}

DevWorker::State DevWorker::state() const {
    return _state;
}

void DevWorker::stateMachine() {
    switch (state()) {
    case State::None: {
        break;
    }
    case State::InitFtp: {
        if (_commander.waitForAnswer()) {
            break;
        }
        QThread::msleep(500);
        Q_ASSERT(_ftpModel.state() == QFtp::Unconnected);
        _ftpModel.connectToHost(ip);
        _ftpModel.login(ftpUsername, ftpPassword);
        _ftpModel.setTransferMode(QFtp::Active);
        _ftpModel.cd("mnt/DCIM");
        _ftpModel.list();
        setState(State::GetFtpList);
        break;
    }
    case State::GetFtpList: {
        if (!_ftpModel.isDone()) {
            break;
        }
        /*
          Тут нужно установить объем для скачивания и начать скачивать.
        */
        break;
    }
    case State::CountState: {
        Q_ASSERT(false);
        break;
    }
    }
}

void DevWorker::setState(State newState)
{
    if (_state == newState)
        return;
    _state = newState;
    emit stateChanged();
}

const QString &DevWorker::ip() const {
    return _ip;
}

const QString &DevWorker::ftpUsername() const {
    return _ftpUsername;
}

const QString &DevWorker::ftpPassword() const {
    return _ftpPassword;
}

const QString &DevWorker::mode() const {
    return _mode;
}

