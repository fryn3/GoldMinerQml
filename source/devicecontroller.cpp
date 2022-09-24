#include "devicecontroller.h"

#include "myfunc.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>
#include <QTimerEvent>

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
    qDebug() << __FILE__ << __LINE__ << "_devWorkers.size()" << _devWorkers.size();
    auto keys = _devWorkers.keys();
    for (const auto & wKey: keys) {
        _devWorkers[wKey]->stopDownloading();
    }
}

void DeviceController::timerEvent(QTimerEvent *event) {
    Q_ASSERT(event->timerId() == __timerIdWait);
    _currentDev = 0;
    killTimer(__timerIdWait);
    __timerIdWait = 0;
    findDev();
}

void DeviceController::findDev() {
    setState(State::FindingDevices);
    _devModel->clear();
    QFile findIpExe(":/resources/soft/FindIP.exe");
    QTemporaryFile *newTempFile = QTemporaryFile::createNativeFile(findIpExe);
    newTempFile->rename(newTempFile->fileName() + ".exe");
    QProcess *p = new QProcess(this);
    connect(p, &QProcess::errorOccurred, this, [this, newTempFile] {
        qDebug() << __FILE__ << __LINE__ << "Не получилось запустить прогу" << newTempFile->fileName();
        setState(State::FatalError);
        sender()->deleteLater();
        newTempFile->remove();
        newTempFile->deleteLater();
        emit finished();
    });

    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            p, &QProcess::deleteLater);
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, newTempFile] {
        newTempFile->remove();
        newTempFile->deleteLater();
        if (_devModel->rowCount() > 0) {
            setState(State::Downloding);
            workerStarting();
        } else {
            setState(State::Wait);
            __timerIdWait = startTimer(_waitTimeMs);
        }
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
            _devModel->addDevice(ip, mac, oName);
        }
    });
    p->start(newTempFile->fileName(), {"VideoServer"});
}

void DeviceController::workerStarting() {
    if (sender()) {
        auto s = qobject_cast<DevWorker*>(sender());
        qDebug() << __FILE__ << __LINE__ << s << s->indexRow;
        _devWorkers.remove(s->indexRow);
        s->deleteLater();
    }
    if (_stoped) {
        if (_devWorkers.isEmpty()) {
            setState(State::None);
            emit finished();
        } else {
            qDebug() << __FILE__ << ":" << __LINE__ << "I'm here" << _devWorkers.size();
        }
        return;
    }
    while (_devWorkers.size() < _countParallel) {
        if (_currentDev >= _devModel->rowCount()) {
            if (_devWorkers.isEmpty()) {
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
        QString timeDir = QDateTime::currentDateTime().toString("yy_MM_dd_HH_mm_ss");

        auto *devWorker = new DevWorker(_currentDev, dStruct, _downloadFolder
                                        , {deviceName, timeDir}, this);

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
    _commander.setVideoRecordMode(mode);

    connect(&_ftpModel, &FtpModel::done, this, &DevWorker::stateMachine);
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
                                            , DeviceCommander::Command::VideoRecordMode
                                            , DeviceCommander::Command::FtpUsername
                                            , DeviceCommander::Command::FtpPassword }));
    Q_ASSERT(error == DeviceCommander::Error::None);
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
}

void DevWorker::stateMachine() {
    if (_stoped) {
        emit finished();
        return;
    }

    if (_commander.error() != DeviceCommander::Error::None) {
        qDebug() << __FILE__ << __LINE__ << _commander.error();
        switch(_commander.error()) {
        case DeviceCommander::Error::CantConnecting:
            _commander.setError(DeviceCommander::Error::None);
            break;
        case DeviceCommander::Error::None:
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
            if (f.isDir()) {
                continue;
            }
            setProgressTotal(progressTotal() + f.size());
        }
        setState(State::GetFtpFiles);
        break;
    }
    case State::GetFtpFiles: {
        if (!_ftpModel.isDone()) {
            break;
        }
        QUrlInfo urlInfo;
        while (!_filesToDownload.isEmpty()) {
            auto url = _filesToDownload.takeFirst();
            if (url.isDir()) {
                qDebug() << __FILE__ << ":" << __LINE__ << "DevWorker::stateMachine" << "wow" << url.name() << "is dir";
                continue;
            }
            urlInfo = std::move(url);
            break;
        }
        if (!urlInfo.isValid()) {
            Q_ASSERT(_filesToDownload.isEmpty());
            qDebug() << __FILE__ << ":" << __LINE__ << "DevWorker::stateMachine" << "Закончили прохождение по файлам";
            setState(State::AllFilesDownloded);
            break;
        }

        QDir selectDir(downloadFolder);
        for (const auto& d: subDirs) {
            auto newDirName = d.trimmed();
            newDirName.replace(QRegExp("[\\\\\\/\\:\\*\\?\\\"<>\\|]"), "_");
            if (!selectDir.cd(newDirName)) {
                selectDir.mkdir(newDirName);
                Q_ASSERT(selectDir.cd(newDirName));
            }
        }

        // возможно нужна проверка и закрытие предыдущего файла.
        if (_file.isOpen()) {
            _file.close();
        }
        _file.setFileName(selectDir.filePath(urlInfo.name()));
        __progressPrevDoneInFile = 0;
        Q_ASSERT(_file.open(QIODevice::WriteOnly));
        _ftpModel.get(urlInfo.name(), &_file);
        _ftpModel.remove(urlInfo.name());
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
