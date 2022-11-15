#include "devicecontroller.h"

#include "myfunc.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>
#include <QTimerEvent>

#include "FindDevice/finddevicecontrollerbase.h"

DeviceController::DeviceController(QObject *parent)
    : QObject{parent} { }

void DeviceController::setDeviceModel(DeviceModel *devModel) {
    _devModel = devModel;
}

void DeviceController::startDownloading() {
    Q_ASSERT(_devModel);
    Q_ASSERT(!downloadFolder().isEmpty());
    Q_ASSERT(_devWorkers.isEmpty());
    emit started();
    setStoped(false);
    _currentDev = 0;
    findDev();
}

void DeviceController::stopDownloading() {
    _stoped = true;
    if (__timerIdWait) {
        killTimer(__timerIdWait);
        __timerIdWait = 0;
    }
    switch (state()) {
    case State::None:
    case State::FatalErrorFindIp:
    case State::FatalErrorBadDir: {
        qDebug() << __FILE__ << __LINE__ << "I'm already stoped";
        break;
    }
    case State::FindingDevices:
    case State::Wait: {
        setState(State::None);
        emit finished();
        break;
    }
    case State::Downloding: {
        auto keys = _devWorkers.keys();
        for (const auto & wKey: keys) {
            _devWorkers[wKey]->stopDownloading();
        }
        break;
    }
    }

}

void DeviceController::timerEvent(QTimerEvent *event) {
    qDebug() << event->timerId() << __timerIdWait;
    Q_ASSERT(event->timerId() == __timerIdWait);
    _currentDev = 0;
    killTimer(__timerIdWait);
    __timerIdWait = 0;
    findDev();
}

void DeviceController::findDev() {
    if (_stoped) {
        return;
    }
    setState(State::FindingDevices);
    _devModel->clear();

    FindDeviceControllerBase *f = FindDeviceControllerBase::createController(this);

    connect(f, &FindDeviceControllerBase::started, [] {
        qDebug() << "FindDeviceControllerBase::started";
    });
    connect(f, &FindDeviceControllerBase::findedDevice, _devModel, &DeviceModel::addDeviceSlot);
    connect(f, &FindDeviceControllerBase::finished, [this, f] {
        if (_stoped) {
            return;
        }
        if (f->isError()) {
            qDebug() << __FILE__ << __LINE__ << QString("Ошибка поиска девайсов: %1").arg(f->errorMsg());
            setState(State::FatalErrorFindIp);
            emit finished();
        } else if (_devModel->rowCount() > 0) {
            setState(State::Downloding);
            workerStarting();
        } else {
            // Если устройств нет, опять 10 секунд ждем!
            setState(State::Wait);
            __timerIdWait = startTimer(_waitTimeMs);
        }
    });
    connect(f, &FindDeviceControllerBase::finished, f, &QObject::deleteLater);
    f->start();
}

void DeviceController::workerStarting() {
    qDebug() << __FILE__ << __LINE__ << _stoped << _devWorkers.size();
    DevWorker *s = nullptr;
    if (sender()) {
        s = qobject_cast<DevWorker*>(sender());
        qDebug() << __FILE__ << __LINE__ << s << s->indexRow << s->error();
        switch (s->error()) {
        case DevWorker::Error::None: {
            break;
        }
        case DevWorker::Error::BadDir: {
             setState(State::FatalErrorBadDir);
             break;
        }
        case DevWorker::Error::FtpSomeError: {
            qDebug() << __FILE__ << __LINE__ << "Произошла не фатальная ошибка. Хз что делать!";
            break;
        }
        default:
            Q_ASSERT(false);
        }
        _devWorkers.remove(s->indexRow);
        s->deleteLater();
    }
    if (_stoped || (s && s->error() == DevWorker::Error::BadDir)) {
        if (_devWorkers.isEmpty()) {
            if (_stoped) {
                setState(State::None);
            }
            emit finished();
        } else {
            qDebug() << __FILE__ << ":" << __LINE__ << "I'm here" << _devWorkers.size();
        }
        return;
    }
    while (_devWorkers.size() < _countParallel) {
        if (_currentDev >= _devModel->rowCount()) {
            if (_devWorkers.isEmpty() && state() != State::Wait) {
                qDebug() << __FILE__ << ":" << __LINE__ << "Wow! I'm waiting!";
                setState(State::Wait);
                __timerIdWait = startTimer(_waitTimeMs);
            }
            qDebug() << __FILE__ << ":" << __LINE__ << "Mmm. I'm here";
            return;
        }
        configFtpServer(_currentDev);

        auto dStruct = _devModel->get(_currentDev,
                                      DeviceModel::DmStructRole).value<DeviceCam>();
        auto deviceName = _devModel->get(_currentDev,
                                         Qt::DisplayRole).toString();
        QString timeDir = QDateTime::currentDateTime().toString("yy_MM_dd");

        auto *devWorker = new DevWorker(_currentDev, dStruct, _downloadFolder
                                        , {deviceName, timeDir});

        connect(devWorker, &DevWorker::finished, this, &DeviceController::workerStarting);
        connect(devWorker, &DevWorker::progressTotalChanged, this, [this, devWorker] {
            _devModel->set(devWorker->indexRow, devWorker->progressTotal(), DeviceModel::DmTotalSizeRole);
        });
        connect(devWorker, &DevWorker::progressDoneChanged, this, [this, devWorker] {
            _devModel->set(devWorker->indexRow, devWorker->progressDone(), DeviceModel::DmDoneSizeRole);
        });
        devWorker->startDownloading();
        _devWorkers.insert(_currentDev, devWorker);
        ++_currentDev;
    }
}

void DeviceController::configFtpServer(int indexDev) {
    Q_ASSERT(!_devWorkers.contains(indexDev));
    QString username = My::genRandom(8);
    QString password = My::genRandom(8);

    _devModel->set(indexDev, username, DeviceModel::DmFtpUsernameRole);
    _devModel->set(indexDev, password, DeviceModel::DmFtpPasswordRole);

}

int DeviceController::countParallel() const {
    return _countParallel;
}

void DeviceController::setCountParallel(int newCountParallel) {
    _countParallel = newCountParallel;
}

int DeviceController::waitTimeMs() const {
    return _waitTimeMs;
}

void DeviceController::setWaitTimeMs(int newWaitTimeMs) {
    _waitTimeMs = newWaitTimeMs;
}

DeviceController::State DeviceController::state() const {
    return _state;
}

void DeviceController::setState(State newState) {
    if (_state == newState)
        return;
    _state = newState;
    emit stateChanged();
}

bool DeviceController::stoped() const {
    return _stoped;
}

void DeviceController::setStoped(bool newStoped) {
    if (_stoped == newStoped)
        return;
    _stoped = newStoped;
    emit stopedChanged();
}

const QString &DeviceController::downloadFolder() const {
    return _downloadFolder;
}

void DeviceController::setDownloadFolder(const QString &newDownloadFolder) {
    if (_downloadFolder == newDownloadFolder)
        return;
    _downloadFolder = newDownloadFolder;
    emit downloadFolderChanged();
}

DevWorker::DevWorker(int index, QString ipStr, QString ftpLog, QString ftpPass, QString folderPath, QStringList subDirsList, QObject *parent)
    : QObject(parent), indexRow(index), ip(ipStr), ftpUsername(ftpLog), ftpPassword(ftpPass),
      downloadFolder(folderPath), subDirs(subDirsList) {

    connect(this, &DevWorker::stateChanged, &DevWorker::stateMachine);

    connect(&_commander, &DeviceCommander::waitForAnswerChanged, this, &DevWorker::stateMachine);
    _commander.setIp(ip);
    _commander.setFtpUsername(ftpUsername);
    _commander.setFtpPassword(ftpPassword);

    connect(&_ftpModel, &FtpModel::done, this, &DevWorker::stateMachine);
    connect(&_ftpModel, &FtpModel::freezeChanged, this, &DevWorker::stateMachine);
    connect(&_ftpModel, &FtpModel::dataTransferProgress, this, [this] (qint64 done, qint64 total) {
        Q_UNUSED(total);
        setProgressDone(progressDone() + done - __progressPrevDoneInFile);
        __progressPrevDoneInFile = done;
    });
}

DevWorker::DevWorker(int index, const DeviceCam &dev, QString folderPath, QStringList subDirs, QObject *parent)
    : DevWorker(index, dev.ip, dev.ftpUsername, dev.ftpPassword, folderPath, subDirs, parent) { }

DevWorker::~DevWorker() noexcept { _ftpModel.abort(); }

DevWorker::State DevWorker::state() const {
    return _state;
}

void DevWorker::startDownloading() {
    emit started();
    _stoped = false;
    auto error = (_commander.sendCommands({ DeviceCommander::Command::SetParameter
                                            , DeviceCommander::Command::FtpUsername
                                            , DeviceCommander::Command::FtpPassword }));
    Q_ASSERT(error == DeviceCommander::Error::NoError);
    setState(State::InitFtp);
}

void DevWorker::stopDownloading()
{
    _stoped = true;
    if (state() == State::None
            || state() == State::InitFtp) {
        emit finished();
        return;
    }
    _ftpModel.abort();
    setState(State::None);
}

void DevWorker::stateMachine() {
    qDebug() << __FILE__ << __LINE__ << error() << _commander.error()
             << _ftpModel.error() << _ftpModel.isDone() << _ftpModel.freeze();
    if (_stoped) {
        emit finished();
        return;
    }

    if (error() != Error::None) {
        emit finished();
        return;
    }

    if (_commander.error() != DeviceCommander::Error::NoError) {
        qDebug() << __FILE__ << __LINE__ << _commander.error();
        switch(_commander.error()) {
        case DeviceCommander::Error::CantConnecting:
            _commander.setError(DeviceCommander::Error::NoError);
            break;
        case DeviceCommander::Error::NoError:
        case DeviceCommander::Error::MissArgument:
        case DeviceCommander::Error::BadArgument:
        case DeviceCommander::Error::WaitForAnswer: {
            qDebug() << _commander.error();
            Q_ASSERT(false);
            // это не сохраняем в поле ошибки а возвращаем через sendCommands().
            break;
        }
        case DeviceCommander::Error::Count:
            Q_ASSERT(false);
            break;
        }
        emit finished();
        return;
    }

    if (_ftpModel.error() || _ftpModel.freeze()) {
        setError(Error::FtpSomeError);
        emit finished();
        return;
    }

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
        setProgressDone(0);
        _filesToDownload = _ftpModel.files().toList();
        if (_filesToDownload.isEmpty()) {
            setProgressTotal(1);
            setProgressDone(1);
            setState(State::AllFilesDownloded);
        }
        for (const auto & f: _filesToDownload) {
            if (f.info.isDir()) {
                continue;
            }
            setProgressTotal(progressTotal() + f.info.size());
        }
        setState(State::GetFtpFiles);
        break;
    }
    case State::GetFtpFiles: {
        if (!_ftpModel.isDone()) {
            break;
        }
        FtpModel::RowStruct rowStruct;
        while (!_filesToDownload.isEmpty()) {
            auto rowStructFirst = _filesToDownload.takeFirst();
            if (rowStructFirst.info.isDir()) {
                qDebug() << __FILE__ << ":" << __LINE__ << "DevWorker::stateMachine" << "wow" << rowStructFirst.info.name() << "is dir";
                continue;
            }
            rowStruct = std::move(rowStructFirst);
            break;
        }
        if (!rowStruct.info.isValid()) {
            Q_ASSERT(_filesToDownload.isEmpty());
            qDebug() << __FILE__ << ":" << __LINE__ << "DevWorker::stateMachine" << "Закончили прохождение по файлам";
            setState(State::AllFilesDownloded);
            break;
        }

        if (onlyRemove()) {
            _ftpModel.remove(rowStruct.info.name());
            break;
        }

        if (rowStruct.info.size() == 0) {
            qDebug() << __FILE__ << __LINE__ << "Удаляем нулевой файл" << rowStruct.info.name();
            _ftpModel.remove(rowStruct.info.name());
            break;
        }

        QDir selectDir(downloadFolder);
        for (const auto& d: subDirs) {
            auto newDirName = d.trimmed();
            newDirName.replace(QRegExp("[\\\\\\/\\:\\*\\?\\\"<>\\|]"), "_");
            if (!selectDir.cd(newDirName)) {
                selectDir.mkdir(newDirName);
                if (!selectDir.cd(newDirName)) {
                    setError(Error::BadDir);
                    setState(State::None);
                    return;
                }
            }
        }

        // возможно нужна проверка и закрытие предыдущего файла.
        if (_file.isOpen()) {
            _file.close();
        }
        _file.setFileName(selectDir.filePath(rowStruct.info.name()));
        __progressPrevDoneInFile = 0;
        if (!_file.open(QIODevice::WriteOnly)) {
            setError(Error::BadDir);
            setState(State::None);
            return;
        }
        _ftpModel.get(rowStruct.info.name(), &_file);
        if (removeAfterDownload()) {
            _ftpModel.remove(rowStruct.info.name());
        }
        break;
    }
    case State::AllFilesDownloded: {
        _file.close();
        setProgressDone(progressTotal());
        emit finished();
        break;
    }
    case State::CountState: {
        Q_ASSERT(false);
        break;
    }
    }
}

void DevWorker::setState(State newState) {
    if (_state == newState)
        return;
    _state = newState;
    emit stateChanged();
}

qint64 DevWorker::progressTotal() const {
    return _progressTotal;
}

void DevWorker::setProgressTotal(qint64 newProgressTotal) {
    if (_progressTotal == newProgressTotal)
        return;
    _progressTotal = newProgressTotal;
    emit progressTotalChanged();
}

qint64 DevWorker::progressDone() const {
    return _progressDone;
}

void DevWorker::setProgressDone(qint64 newProgressDone) {
    if (_progressDone == newProgressDone)
        return;
    _progressDone = newProgressDone;
    emit progressDoneChanged();
}

DevWorker::Error DevWorker::error() const {
    return _error;
}

void DevWorker::setError(Error newError) {
    if (_error == newError)
        return;
    _error = newError;
    emit errorChanged();
}

bool DevWorker::onlyRemove() const {
    return _onlyRemove;
}

void DevWorker::setOnlyRemove(bool newOnlyRemove) {
    _onlyRemove = newOnlyRemove;
}

bool DevWorker::removeAfterDownload() const {
    return _removeAfterDownload;
}

void DevWorker::setRemoveAfterDownload(bool newRemoveAfterDownload) {
    _removeAfterDownload = newRemoveAfterDownload;
}
