#include "devicecontroller.h"

#include "myfunc.h"

#include <QDir>
#include <QThread>

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
    _currentDev = 0;
    workerStarting();
}

void DeviceController::workerStarting() {
    if (sender()) {
        auto s = qobject_cast<DevWorker*>(sender());
        _devWorkers.remove(s->indexRow);
        s->deleteLater();
    }
    while (_devWorkers.size() < _countParallel) {
        if (_currentDev >= _devModel->rowCount()) {
            qDebug() << "В моделе девайсы закончились.";
            emit  finished();
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
            qDebug() << "DeviceController total" << devWorker->progressTotal();
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

DevWorker::State DevWorker::state() const {
    return _state;
}

void DevWorker::startDownloading() {
    emit started();
    auto error = (_commander.sendCommands({ DeviceCommander::Command::SetParameter
                                            , DeviceCommander::Command::VideoRecordMode
                                            , DeviceCommander::Command::FtpUsername
                                            , DeviceCommander::Command::FtpPassword }));
    Q_ASSERT(error == DeviceCommander::Error::None);
    setState(State::InitFtp);
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
        _filesToDownload = _ftpModel.files().toList();
        setProgressDone(0);
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
                qDebug() << "DevWorker::stateMachine" << "wow" << url.name() << "is dir";
                continue;
            }
            urlInfo = std::move(url);
            break;
        }
        if (!urlInfo.isValid()) {
            Q_ASSERT(_filesToDownload.isEmpty());
            qDebug() << "DevWorker::stateMachine" << "Закончили прохождение по файлам";
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
//        _ftpModel.remove(urlInfo.name()); /// \todo uncomment me
        break;
    }
    case State::AllFilesDownloded: {
        _file.close();
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
