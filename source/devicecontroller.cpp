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
    Q_ASSERT(_devsCommander.isEmpty());
    _currentDev = 0;
}

void DeviceController::workerStarting() {
    while (_devsCommander.size() < _countParallel) {
        if (_currentDev >= _devModel->rowCount()) {
            qDebug() << "В моделе девайсы закончились.";
            return;
        }
        configFtpServer(_currentDev);

        auto dStruct = _devModel->get(_currentDev,
                                      DeviceModel::DmStructRole).value<DeviceCam>();
        auto deviceName = _devModel->get(_currentDev,
                                         Qt::DisplayRole).toString();

        auto *devWorker = new DevWorker(dStruct, _downloadFolder, deviceName, this);



        devWorker->startDownloading();
        _devsCommander.insert(_currentDev, devWorker);
    }
}

void DeviceController::configFtpServer(int indexDev) {
    Q_ASSERT(!_devsCommander.contains(indexDev));
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

DevWorker::DevWorker(QString ipStr, QString ftpLog, QString ftpPass, QString folderPath, QString name, QObject *parent)
    : QObject(parent), ip(ipStr), ftpUsername(ftpLog), ftpPassword(ftpPass),
      downloadFolder(folderPath), dirName(name) {

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

DevWorker::DevWorker(const DeviceCam &dev, QString folderPath, QString name, QObject *parent)
    : DevWorker(dev.ip, dev.ftpUsername, dev.ftpPassword, folderPath, name, parent) { }

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

        if (!selectDir.cd(dirName)) {
            selectDir.mkdir(dirName);
            Q_ASSERT(selectDir.cd(dirName));
        }

        // возможно нужна проверка и закрытие предыдущего файла.
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

int DevWorker::progressTotal() const {
    return _progressTotal;
}

void DevWorker::setProgressTotal(int newProgressTotal) {
    if (_progressTotal == newProgressTotal)
        return;
    _progressTotal = newProgressTotal;
    emit progressTotalChanged();
}

int DevWorker::progressDone() const {
    return _progressDone;
}

void DevWorker::setProgressDone(int newProgressDone) {
    if (_progressDone == newProgressDone)
        return;
    _progressDone = newProgressDone;
    emit progressDoneChanged();
}
