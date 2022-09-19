#include "core.h"
#include "myfunc.h"
#include "qcoreapplication.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QThread>

const QString Core::ITEM_NAME = "Core";
static void regT() {
    My::qmlRegisterType<Core>(Core::ITEM_NAME);
}
Q_COREAPP_STARTUP_FUNCTION(regT)
const bool Core::IS_QML_REG = true;//My::qmlRegisterType<Core>(Core::ITEM_NAME);

Core::Core(QObject *parent)
    : QObject{parent} {

    connect(this, &Core::devModelCurrentIndexChanged, [] {
        qDebug() << "Core::devModelCurrentIndexChanged";
    });
    connect(this, &Core::stateChanged, [] {
        qDebug() << "Core::stateChanged";
    });

    connect(&_devCommander, &DeviceCommander::waitForAnswerChanged, [] {
        qDebug() << "DeviceCommander::waitForAnswerChanged";
    });
    connect(ftpModel(), &FtpModel::errorChanged, [] {
        qDebug() << "FtpModel::errorChanged";
    });
    connect(ftpModel(), &FtpModel::done, [] {
        qDebug() << "FtpModel::done";
    });
    connect(this, &Core::devModelCurrentIndexChanged, this, [this] {
        if (ftpModel()->state() != QFtp::Unconnected) {
            ftpModel()->close();
        }
    });
    connect(this, &Core::devModelCurrentIndexChanged, this, &Core::updateCurrentDeviceCam, Qt::DirectConnection);
    connect(this, &Core::devModelCurrentIndexChanged, this, &Core::stateMachine);
    connect(this, &Core::stateChanged, this, &Core::stateMachine);

    connect(&_devCommander, &DeviceCommander::waitForAnswerChanged, this, &Core::stateMachine);
    connect(ftpModel(), &FtpModel::errorChanged, this, &Core::stateMachine);
    connect(ftpModel(), &FtpModel::done, this, &Core::stateMachine);

    connect(&_devModel, &DeviceModel::dataChanged, this, [this]
            (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
        qDebug() << "DeviceModel::dataChanged" << topLeft.row();
        if (topLeft.row() <= devModelCurrentIndex()
                && bottomRight.row() >= devModelCurrentIndex()) {
            updateCurrentDeviceCam();
        }
    }, Qt::DirectConnection);

    _devController.setDeviceModel(devModel());

}

void Core::findDev() {
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
        setState(State::None);
    });
    p->start(newTempFile->fileName(), {"VideoServer"});
    emit showMessage("Поиск устройств...");
}

void Core::initFtpServer() {
    QString username = currentDeviceCam().ftpUsername; //devModel()->get(devModelCurrentIndex(), DeviceModel::DmFtpUsernameRole).toString();
    QString password = currentDeviceCam().ftpPassword; //devModel()->get(devModelCurrentIndex(), DeviceModel::DmFtpPasswordRole).toString();

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
                                            , DeviceCommander::Command::VideoRecordMode
                                            , DeviceCommander::Command::FtpUsername
                                            , DeviceCommander::Command::FtpPassword }));
    Q_ASSERT(error == DeviceCommander::Error::None);

}

DeviceController *Core::devController() {
    return &_devController;
}

void Core::showFtpFiles() {
    initFtpServer();
    setState(State::ShowFtpFilesInitFtp);
}

void Core::readDevConfig() {
    initFtpServer();
    setState(State::ReadingConfigInitFtp);
}

void Core::writeDevConfig() {
    _devCommander.setData(currentDeviceCam());

    Q_ASSERT(_devCommander.sendCommands() == DeviceCommander::Error::None);

    setState(State::WriteingConfigWaitTcp);
}

void Core::updateCurrentDeviceCam() {
    auto d = devModel()->get(devModelCurrentIndex(),
                             DeviceModel::DmStructRole).value<DeviceCam>();
    if (d != currentDeviceCam()) {
        setCurrentDeviceCam(d);
    }
}

void Core::stateMachine() {

    qDebug() << __FILE__ << __LINE__ << state();

    if (ftpModel()->error()) {
        qDebug() << ftpModel()->errorString();
        emit showMessage(ftpModel()->errorString());
        if (ftpModel()->state() != QFtp::Unconnected) {
            ftpModel()->close();
        }
//        Q_ASSERT(false);
        return;
    }

    switch (state()) {
    case State::None: {
        break;
    }
    case State::FindingDevices: {
        break;
    }
    case State::ShowFtpFilesInitFtp: {
        if (_devCommander.waitForAnswer()) {
            break;
        }
        QThread::sleep(1);
        Q_ASSERT(ftpModel()->state() == QFtp::Unconnected);
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
        emit showMessage("Должен показаться список файлов", 5000);
        break;
    }
    case State::ReadingConfigInitFtp: {
        if (_devCommander.waitForAnswer()) {
            break;
        }
        QThread::sleep(1);
        if (ftpModel()->state() != QFtp::Unconnected) {
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
        qDebug() << " settIni = " << settIni;
        _devModel.parseSettingsIni(settIni, devModelCurrentIndex());
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
    return &_ftpModel;
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
    qDebug() << "currentIndex = " << newDevModelCurrentIndex;
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
