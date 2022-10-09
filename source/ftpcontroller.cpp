#include "ftpcontroller.h"

#include <QDebug>
#include <QDir>
#include <QFile>

FtpController::FtpController(QObject *parent)
    : QObject{parent} { }

bool FtpController::removeAfterDownload() const {
    return _removeAfterDownload;
}

void FtpController::setRemoveAfterDownload(bool newRemoveAfterDownload) {
    _removeAfterDownload = newRemoveAfterDownload;
}

void FtpController::startDownloadAll(QString path) {
    _state = State::Downloading;
    _path = path;
    prepareProcess();
}

void FtpController::removeAll() {
    _state = State::Removing;
    prepareProcess();
}

void FtpController::stop() {
    _state = State::Stoping;
    if (_ftpModel->currentCommand() == QFtp::Get) {
        emit stoping();
    }
}

void FtpController::commandStartedSlot(int i) {
    qDebug() << "started" << i;
}

void FtpController::commandFinishedSlot(int i, bool e) {
    qDebug() << "finished" << i << e;
}

void FtpController::ftpDone() {
    qDebug() << "FtpController::ftpDone" << _ftpModel->isDone() << _ftpModel->error();
    if (_ftpModel->error()) {
        qDebug() << "FtpController::ftpDone error" << _ftpModel->errorString();
        return finish();
    }
    if (_ftpModel->freeze()) {
        qDebug() << "FtpController::ftpDone freeze";
        return finish();
    }
    if (++__indexDownload >= _files->size() || _state == State::Stoping) {
        return finish();
    }

    startNext();
}

void FtpController::startNext() {
    qDebug() << "FtpController::startNext";

    const auto &f = _files->at(__indexDownload);
    if (f.info.size() == 0) {
        qDebug() << __FILE__ << __LINE__ << "Удаляем нулевой файл" << f.info.name();
        _ftpModel->remove(f.info.name());
        return;
    }
    switch (_state) {
    case State::None:
    case State::Stoping:
        Q_ASSERT(false);
        return;
    case State::Downloading: {
        QDir selectDir(_path);
        // deviceName, timeDir
        QStringList subDirs = {_deviceName, QDate::currentDate().toString("yy_MM_dd")};

        for (const auto& d: subDirs) {
            auto newDirName = d.trimmed();
            newDirName.replace(QRegExp("[\\\\\\/\\:\\*\\?\\\"<>\\|]"), "_");
            if (!selectDir.cd(newDirName)) {
                selectDir.mkdir(newDirName);
                if (!selectDir.cd(newDirName)) {
                    qDebug() << "Не смог создать папку" << selectDir.dirName();
                    return finish();
                }
            }
        }

        _getFile = new QFile(selectDir.filePath(f.info.name()));
        if (!_getFile->open(QIODevice::WriteOnly)) {
            qDebug() << __FILE__ << __LINE__ << "Can't open file" << _getFile->fileName();
            return finish();
        }
        _ftpModel->get(f.info.name(), _getFile);
        if (_removeAfterDownload) {
            _ftpModel->remove(f.info.name());
        }
        break;
    }
    case State::Removing: {
        _ftpModel->remove(f.info.name());
        break;
    }
    }
}

void FtpController::prepareProcess() {
    ftpConnect();
    _files = &_ftpModel->files();
    __indexDownload = 0;
    emit started();
    startNext();
}

void FtpController::finish()
{
    if (_getFile) {
        _getFile->deleteLater();
    }
    emit finished();
}

void FtpController::ftpConnect() {
    connect(_ftpModel, &FtpModel::done, this, &FtpController::ftpDone);
    connect(_ftpModel, &FtpModel::freezeChanged, this, &FtpController::ftpDone);
    connect(this, &FtpController::finished, this, &FtpController::ftpDisconnect);
    connect(this, &FtpController::stoping, _ftpModel, &FtpModel::abort);
}

void FtpController::ftpDisconnect() {
    disconnect(_ftpModel, &FtpModel::done, this, &FtpController::ftpDone);
    disconnect(_ftpModel, &FtpModel::freezeChanged, this, &FtpController::ftpDone);
    disconnect(this, &FtpController::finished, this, &FtpController::ftpDisconnect);
    disconnect(this, &FtpController::stoping, _ftpModel, &FtpModel::abort);
}

FtpController::State FtpController::state() const {
    return _state;
}

const QString &FtpController::deviceName() const {
    return _deviceName;
}

void FtpController::setDeviceName(const QString &newDeviceName) {
    _deviceName = newDeviceName;
}

FtpModel *FtpController::ftpModel() const {
    return _ftpModel;
}

void FtpController::setFtpModel(FtpModel *newFtpModel) {
    _ftpModel = newFtpModel;
}
