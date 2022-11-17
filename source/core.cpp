#include "core.h"
#include "myfunc.h"
#include "qcoreapplication.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>

#include "FindDevice/finddevicecontrollerbase.h"

const QString Core::ITEM_NAME = "Core";
static void regT() {
    My::qmlRegisterType<Core>(Core::ITEM_NAME);
}
Q_COREAPP_STARTUP_FUNCTION(regT)
const bool Core::IS_QML_REG = true;//My::qmlRegisterType<Core>(Core::ITEM_NAME);

Core::Core(QObject *parent)
    : QObject{parent}, _ftpModel{nullptr} {

    connect(this, &Core::devModelCurrentIndexChanged, this, [this] {
        qDebug() << ftpModel()->state();
        if (ftpModel()->state() != QFtp::Unconnected) {
            ftpModel()->close();
        }
    });
    connect(this, &Core::devModelCurrentIndexChanged
            , this, &Core::updateCurrentDeviceCam, Qt::DirectConnection);
    connect(this, &Core::devModelCurrentIndexChanged, &Core::stateMachine);
    connect(this, &Core::stateChanged, &Core::stateMachine);

    connect(&_devCommander, &DeviceCommander::waitForAnswerChanged, this, &Core::stateMachine);

    ftpReconnect();

    connect(devModel(), &DeviceModel::dataChanged, this, [this]
            (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        if (topLeft.row() <= devModelCurrentIndex()
                && bottomRight.row() >= devModelCurrentIndex()) {
            updateCurrentDeviceCam();
        }
    }, Qt::DirectConnection);
    connect(devModel(), &DeviceModel::rowCountChanged, this, &Core::devicesFoundChanged);

    devController()->setDeviceModel(devModel());

    connect(devController(), &DeviceController::downloadFolderChanged
            , this, &Core::deviceControllerPathChanged);

    connect(devController(), &DeviceController::finished, [this] {
        Q_ASSERT(devController()->state() == DeviceController::State::None
                 || devController()->state() == DeviceController::State::FatalErrorFindIp
                 || devController()->state() == DeviceController::State::FatalErrorBadDir);
        setState(State::None);
    });

    connect(devController(), &DeviceController::stateChanged
            , this, [this] {
        switch (devController()->state()) {
        case DeviceController::State::None: {
            Q_ASSERT(devController()->stoped() == true);
            devModel()->setProgressBarZero();
            emit showMessage("Автозагрузка остановлена");
            break;
        }
        case DeviceController::State::FindingDevices: {
            emit showMessage("Автозагрузка: идет поиск устройств");
            break;
        }
        case DeviceController::State::Downloding: {
            emit showMessage("Автозагрузка: идет загрузка файлов");
            break;
        }
        case DeviceController::State::Wait: {
            emit showMessage(QString("Автозагрузка: через %1 секунд начнется поиск устройств")
                             .arg(devController()->waitTimeMs() / 1000));
            break;
        }
        case DeviceController::State::FatalErrorFindIp: {
            emit showMessage("Автозагрузка остановлена! Ошибка при получинии списка устройств");
            break;
        }
        case DeviceController::State::FatalErrorBadDir: {
            emit showMessage("Автозагрузка остановлена! Плохой путь для загрузки");
        }
        }
    });

    switch (_config.error()) {
    case ConfigController::Error::NoError: {
        devController()->setCountParallel(_config.countParallel);
        devController()->setDownloadFolder(_config.autoDownloadingPath);
        devModel()->setMacAndName(_config.devices);
        break;
    }
    case ConfigController::Error::BadJsonFormat: {
        emit showMessage(QString("Ошибка формата в %1").arg(_config.CONFIG_FILE));
        break;
    }
    case ConfigController::Error::BadParallelValue: {
        emit showMessage(QString("Плохое значение %1 ключа")
                                        .arg(_config.KEY_COUNT_PARALLEL));
        break;
    }
    case ConfigController::Error::CantOpenFile: {
        emit showMessage(QString("Не смог открыть файл конфигов %1")
                                            .arg(_config.CONFIG_FILE));
        break;
    }
    } // _config.error()

    connect(&_ftpController, &FtpController::finished, this, [this] {
        setState(State::None);
    });
}

Core::~Core() noexcept {
    ftpModel()->abort();
    _config.devices.insert(devModel()->macAndName());
}

void Core::findDev() {
    setState(State::FindingDevices);
    devModel()->clear();

    FindDeviceControllerBase *f = FindDeviceControllerBase::createController(this);

    connect(f, &FindDeviceControllerBase::started, [] {
        qDebug() << "FindDeviceControllerBase::started";
    });
    connect(f, &FindDeviceControllerBase::findedDevice, devModel(), &DeviceModel::addDeviceSlot);
    connect(f, &FindDeviceControllerBase::finished, [this, f] {
        if (f->isError()) {
            emit showMessage(QString("Ошибка поиска девайсов: %1")
                                       .arg(f->errorMsg()), -1);
        } else if (devModel()->rowCount() > 0) {
            emit showMessage(QString("Поиск завершен. Найдено %1 устройства!")
                                                    .arg(devModel()->rowCount()), 5000);
        } else {
            emit showMessage("Поиск завершен... Устройства не найдены!", 5000);
        }
        setState(State::None);
    });
    connect(f, &FindDeviceControllerBase::finished, f, &QObject::deleteLater);
    f->start();
    emit showMessage("Поиск устройств...");
}

void Core::runAutoDownloading() {
    if (deviceControllerPath().isEmpty()) {
        emit showMessage(QString("Ошибка! Установите папку для скачивания в %1!")
                                        .arg(_config.CONFIG_FILE));
        return;
    }
    emit showMessage("Начало автоскачивания", 5000);
    setState(State::ProcessAutoDownloading);
    devController()->startDownloading();
}

void Core::stopAutoDownloading() {
    emit showMessage("Идет процесс остановки автоскачивания!");
    setState(State::StoppingAutoDownloading);
    devController()->stopDownloading();
}

void Core::initFtpServer() {
    qDebug() << "Core::initFtpServer" << ftpModel()->state();
    if (ftpModel()->state() == QFtp::State::Connected
            || ftpModel()->state() == QFtp::State::LoggedIn) {
        ftpModel()->close();
    }

    QString username = currentDeviceCam().ftpUsername;
    QString password = currentDeviceCam().ftpPassword;

    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "genRandom";
        username = My::genRandom(8);
        password = My::genRandom(8);
        devModel()->set(devModelCurrentIndex(), username, DeviceModel::DmFtpUsernameRole);
        devModel()->set(devModelCurrentIndex(), password, DeviceModel::DmFtpPasswordRole);
    }
    qDebug() << "FTP AUTH: " << username << "\t" << password;

    _devCommander.setData(currentDeviceCam());

    auto error = (_devCommander.sendCommands({ DeviceCommander::Command::SetParameter
                                            , DeviceCommander::Command::FtpUsername
                                            , DeviceCommander::Command::FtpPassword }));
    Q_ASSERT(error == DeviceCommander::Error::NoError);

}

bool Core::someDebugFlag() const {
    return _someDebugFlag;
}

void Core::setSomeDebugFlag(bool newSomeDebugFlag) {
    if (_someDebugFlag == newSomeDebugFlag)
        return;
    _someDebugFlag = newSomeDebugFlag;
    emit someDebugFlagChanged();
}

DeviceController *Core::devController() {
    return &_devController;
}

QString Core::deviceControllerPath() const {
    return _devController.downloadFolder();
}

void Core::showFtpFiles() {
    Q_ASSERT(ftpModel()->isDone());
    Q_ASSERT(!_devCommander.waitForAnswer());
    emit showMessage("Получения списка файлов");
    initFtpServer();
    setState(State::ShowFtpFilesInitFtp);
}

void Core::readDevConfig() {
    Q_ASSERT(ftpModel()->isDone());
    Q_ASSERT(!_devCommander.waitForAnswer());
    emit showMessage("Чтение настроек");
    initFtpServer();
    setState(State::ReadingConfigInitFtp);
}

void Core::writeDevConfig() {
    if (_devCommander.waitForAnswer()) {
        emit showMessage("TCP не дождались ответа!");
    }
    _devCommander.setData(currentDeviceCam());

    if (_devCommander.sendCommands() != DeviceCommander::Error::NoError) {
        emit showMessage("TCP ошибка!");
    }

    setState(State::WriteingConfigWaitTcp);
}

void Core::updateCurrentDeviceCam() {
    auto d = devModel()->get(devModelCurrentIndex(),
                             DeviceModel::DmStructRole).value<DeviceCam>();
    if (d != currentDeviceCam()) {
        setCurrentDeviceCam(d);
    }
}

void Core::cleanDevice() {
    _ftpController.setFtpModel(ftpModel());
    _ftpController.removeAll();
    setState(State::ProcessRemoving);
}

void Core::stopCleanDevice() {
    _ftpController.stop();
    setState(State::ProcessStoping);
}

void Core::downloadDevice(QString pathDir, bool removeAfterDownlod) {
    qDebug() << __LINE__ << pathDir;
    _ftpController.setFtpModel(ftpModel());
    _ftpController.setDeviceName(_devModel.get(_devModelCurrentIndex).toString());
    _ftpController.setRemoveAfterDownload(removeAfterDownlod);
    _ftpController.startDownloadAll(pathDir);
    setState(State::ProcessDownloading);
}

void Core::stopDownloadDevice() {
    _ftpController.stop();
    setState(State::ProcessStoping);
}

void Core::ftpReconnect() {
    if (_ftpModel) {
        _ftpModel->deleteLater();
    }
    _ftpModel = new FtpModel(this);
    emit ftpModelChanged();
    connect(ftpModel(), &FtpModel::freezeChanged, this, &Core::ftpReconnect);
    connect(ftpModel(), &FtpModel::errorChanged, this, &Core::stateMachine);
    connect(ftpModel(), &FtpModel::done, this, &Core::stateMachine);
}

void Core::stateMachine() {

    qDebug() << __FILE__ << __LINE__ << state() << _devCommander.error() << ftpModel()->error();
    if (ftpModel()->freeze()) {
        qDebug() << ftpModel()->freeze() << ftpModel()->state() << ftpModel()->isDone();
    }

    if (_devCommander.error() != DeviceCommander::Error::NoError) {
        qDebug() << __FILE__ << __LINE__ << _devCommander.error();
        switch(_devCommander.error()) {
        case DeviceCommander::Error::CantConnecting:
            _devCommander.setError(DeviceCommander::Error::NoError);
            emit showMessage("Не смог подключиться к хосту!");
            break;
        case DeviceCommander::Error::NoError:
        case DeviceCommander::Error::MissArgument:
        case DeviceCommander::Error::BadArgument:
        case DeviceCommander::Error::WaitForAnswer: {
            qDebug() << _devCommander.error();
            Q_ASSERT(false);
            // это не сохраняем в поле ошибки а возвращаем через sendCommands().
            break;
        }
        case DeviceCommander::Error::Count:
            Q_ASSERT(false);
            break;
        }
        setState(State::None);
        return;
    }

    if (ftpModel()->error()) {
        qDebug() << __FILE__ << ":" << __LINE__ << ftpModel()->errorString();
        emit showMessage(ftpModel()->errorString());
        if (ftpModel()->state() != QFtp::Unconnected
                && ftpModel()->state() != QFtp::Closing) {
            ftpModel()->close();
        }
        setState(State::None);
        return;
    }

    switch (state()) {
    case State::None:
    case State::FindingDevices:
    case State::ProcessAutoDownloading:
    case State::StoppingAutoDownloading:
    case State::ProcessDownloading:
    case State::ProcessRemoving:
    case State::ProcessStoping: {
        break;
    }
    case State::ShowFtpFilesInitFtp: {
        if (_devCommander.waitForAnswer()) {
            break;
        }
        QThread::msleep(500);
        if (ftpModel()->state() != QFtp::Unconnected) {
            QThread::msleep(500);
        }
//        Q_ASSERT(ftpModel()->state() == QFtp::Unconnected);
        ftpModel()->connectToHost(currentDeviceCam().ip);
        ftpModel()->login(currentDeviceCam().ftpUsername,
                        currentDeviceCam().ftpPassword);
        ftpModel()->setTransferMode(QFtp::Active);
        ftpModel()->cd("mnt/DCIM");
        ftpModel()->list();
        setState(State::ShowFtpFilesGetList);
        break;
    }
    case State::ShowFtpFilesGetList: {
        if (!ftpModel()->isDone()) {
            break;
        }
        if (ftpModel()->rowCount()) {
            emit showMessage(QString("Получен список файлов (%1)!").arg(ftpModel()->rowCount()), 5000);
        } else {
            emit showMessage("На устройстве нет файлов");
        }
        setState(State::None);
        break;
    }
    case State::ReadingConfigInitFtp: {
        if (_devCommander.waitForAnswer()) {
            break;
        }
        QThread::sleep(1);
        if (ftpModel()->state() != QFtp::Unconnected
                && ftpModel()->state() != QFtp::Closing) {
            qDebug() << __FILE__ << __LINE__ << ftpModel()->state();
            ftpModel()->close();
        }
        ftpModel()->connectToHost(currentDeviceCam().ip);
        ftpModel()->login(currentDeviceCam().ftpUsername,
                        currentDeviceCam().ftpPassword);
        ftpModel()->setTransferMode(QFtp::Active);
        ftpModel()->cd("mnt");
        ftpModel()->list();
        setState(State::ReadingConfigWaitFtp);
        break;
    }
    case State::ReadingConfigWaitFtp: {
        if (!ftpModel()->isDone()) {
            break;
        }
        const QString settFile{"settings.ini"};
        auto indexF = ftpModel()->findName(settFile);
        if (indexF == -1) {
            emit showMessage("Не найден файл настроек");
            return;
        }
        if (_settingsFile) {
            _settingsFile->deleteLater();
        }
        _settingsFile = new QTemporaryFile(this);
        _settingsFile->open();
        ftpModel()->get(settFile, _settingsFile);
        setState(State::ReadingConfigDownloading);
        break;
    }
    case State::ReadingConfigDownloading: {
        if (!ftpModel()->isDone()) {
            break;
        }
        ftpModel()->close();
        _settingsFile->seek(0);
        auto settIni = _settingsFile->readAll();
        qDebug() << __FILE__ << ":" << __LINE__ << " settIni = " << settIni;
        devModel()->parseSettingsIni(settIni, devModelCurrentIndex());
        emit showMessage("Прочитал настройки", 5000);
        setState(State::None);
        break;
    }
    case State::WriteingConfigWaitTcp: {
        if (_devCommander.waitForAnswer()) {
            break;
        }
        emit showMessage("Записал настройки", 5000);
        setState(State::None);
        break;
    }
    case State::CountState: {
        Q_ASSERT(false);
        break;
    }
    }
}

FtpModel *Core::ftpModel() {
    return _ftpModel;
}

DeviceCam Core::currentDeviceCam() const {
    return _currentDeviceCam;
}

void Core::setCurrentDeviceCam(DeviceCam newCurrentDeviceCam) {
    if (_currentDeviceCam == newCurrentDeviceCam)
        return;
    _currentDeviceCam = newCurrentDeviceCam;
    emit currentDeviceCamChanged();
}

void Core::resetCurrentDeviceCam() {
    setCurrentDeviceCam({});
}

int Core::devModelCurrentIndex() const {
    return _devModelCurrentIndex;
}

void Core::setDevModelCurrentIndex(int newDevModelCurrentIndex) {
    qDebug() << __FILE__ << ":" << __LINE__ << "currentIndex = " << newDevModelCurrentIndex;
    if (_devModelCurrentIndex == newDevModelCurrentIndex)
        return;
    _devModelCurrentIndex = newDevModelCurrentIndex;
    emit devModelCurrentIndexChanged();
}

void Core::resetDevModelCurrentIndex() {
    setDevModelCurrentIndex(-1);
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

DeviceModel *Core::devModel() {
    return &_devModel;
}

int Core::devicesFound() const {
    return _devModel.rowCount();
}
