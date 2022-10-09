#include "devicecommander.h"
#include <map>
#include "myfunc.h"
#include "qcoreapplication.h"

#include <QSet>
#include <QTcpSocket>
#include <QDateTime>

const QString DeviceCommander::ITEM_NAME = "DeviceCommander";
static void regT() {
    My::qmlRegisterType<DeviceCommander>(DeviceCommander::ITEM_NAME);
}
Q_COREAPP_STARTUP_FUNCTION(regT)
const bool DeviceCommander::IS_QML_REG = true;//My::qmlRegisterType<DeviceCommander>(DeviceCommander::ITEM_NAME);

const QMap<DeviceCommander::Command, QString> DeviceCommander::COMMAND_PATTERN {
    { DeviceCommander::Command::SetParameter, "SET_PARAMETER rtsp://%1:%2/user=%3&password=%4 RTSP/1.0\r\n" },
    { DeviceCommander::Command::Date, "Date: %1\r\n" },
    { DeviceCommander::Command::ContentType, "Content-type: text/rtsp-parameters\r\n" },
    { DeviceCommander::Command::UniqueId, "UNIQUE_ID: %1\r\n" },
    { DeviceCommander::Command::StatusString, "STATUS_STRING: %1\r\n" },
    { DeviceCommander::Command::VideoRecordMode, "VIDEO_RECORD_MODE: %1\r\n" },
    { DeviceCommander::Command::VideoDuration, "VIDEO_DURATION: %1\r\n" },
    { DeviceCommander::Command::ChargeDetectDelay, "CHARGE_DETECT_DELAY: %1\r\n" },
    { DeviceCommander::Command::LogWrite, "LOG_WRITE: %1\r\n" },
    { DeviceCommander::Command::VideoRotation, "VIDEO_ROTATION: %1\r\n" },
    { DeviceCommander::Command::FtpUsername, "FTP_USERNAME: %1\r\n" },
    { DeviceCommander::Command::FtpPassword, "FTP_PASSWORD: %1\r\n" },
};


DeviceCommander::DeviceCommander(QObject *parent)
    : QObject{parent} {

    auto f = [this] () {
        Q_ASSERT(waitForAnswer() == false);
        if (_socket) {
            disconnect(_socket);
            _socket->deleteLater();
            _socket = nullptr;
            setError(Error::NoError);
        }
        setWaitForAnswer(false);
    };

    connect(this, &DeviceCommander::ipChanged, f);
    connect(this, &DeviceCommander::portChanged, f);
    connect(this, &DeviceCommander::rtspLoginChanged, f);
    connect(this, &DeviceCommander::rtspPasswordChanged, f);
    connect(this, &DeviceCommander::uniqueIdChanged, f);
    connect(this, &DeviceCommander::statusStringChanged, f);
    connect(this, &DeviceCommander::videoRecordModeChanged, f);
    connect(this, &DeviceCommander::videoDurationChanged, f);
    connect(this, &DeviceCommander::chargeDetectDelayChanged, f);
    connect(this, &DeviceCommander::logWriteChanged, f);
    connect(this, &DeviceCommander::videoRotationChanged, f);
    connect(this, &DeviceCommander::ftpUsernameChanged, f);
    connect(this, &DeviceCommander::ftpPasswordChanged, f);

}

void DeviceCommander::setData(DeviceCam d) {
    setIp(d.ip);
    setUniqueId(d.uniqueId);
    setStatusString(d.statusString);
    setVideoRecordMode(QString("%1").arg(d.mode));
    setVideoDuration(QString("%1").arg(d.videoDuration));
    setChargeDetectDelay(QString("%1").arg(d.chargeDetectDelay));
    setLogWrite(QString("%1").arg(int(d.logWrite)));
    setVideoRotation(QString("%1").arg(int(d.videoRotation)));
    setFtpUsername(d.ftpUsername);
    setFtpPassword(d.ftpPassword);
}

DeviceCommander::Error DeviceCommander::sendCommands(const QSet<Command> &commands) {
    qDebug() << __LINE__;
    setError(Error::NoError);
    if (waitForAnswer()) {
        qDebug() << "Error::WaitForAnswer";
        return Error::WaitForAnswer;
    }
    QMap<Command, QString> map;
    if (commands.isEmpty()) {
        // Если массив пустой, отправляем все команды
        for (int c = 0; c < int(Command::Count); ++c) {
            map.insert(Command(c), COMMAND_PATTERN[Command(c)]);
        }
    } else {
        for (const auto &command: commands) {
            if (command == Command::Count) {
                qDebug() << "Error::WaitBadArgument";
                return Error::BadArgument;
            }
            map.insert(command, COMMAND_PATTERN[command]);
        }
    }
    // Вместе с каждой командой отправляем время!
    map.insert(Command::Date, COMMAND_PATTERN[Command::Date]);
    _sendMsg.clear();
    for (auto pairIt = map.constBegin(); pairIt != map.constEnd(); ++pairIt) {
        auto args = argsFunc(pairIt.key());
        auto commandStr = pairIt.value();
        for (const auto& a: args) {
            if (a.isEmpty() && pairIt.key() != Command::ContentType) {
                qDebug() << __FILE__ << ":" << __LINE__ << commandStr << args;
                return Error::MissArgument;
            }
            commandStr = commandStr.arg(a);
        }
        _sendMsg.append(commandStr.toLatin1());
    }
    _sendMsg.append("\r\n");
    qDebug() << "SEND_COMMAND ### " << _sendMsg;

    if (_socket) {
        disconnect(_socket);
        _socket->deleteLater();
    }
    _socket = new QTcpSocket(this);
    qDebug() << __FILE__ << __LINE__ << "socet state = " << _socket->state();
    connect(_socket, &QTcpSocket::connected, [] {
        qDebug() << __FILE__ << ":" << __LINE__ << "Connected";
    });
    connect(_socket, &QTcpSocket::disconnected, this, [this] {
        qDebug() << __FILE__ << ":" << __LINE__ << "Disconnected" << sender();
    });
    connect(_socket, &QTcpSocket::bytesWritten, [] (qint64 b) {
        qDebug() << __FILE__ << ":" << __LINE__ << "bytesWritten" << b;
    });
    connect(_socket, &QTcpSocket::readyRead, this, &DeviceCommander::socketReadyRead);
    connect(_socket, &QTcpSocket::stateChanged, this, &DeviceCommander::socketStateChanged);
    __timerId = startTimer(_timeOut);
    _socket->connectToHost(ip(), port().toInt());
    setWaitForAnswer(true);

    socketSendMsg();

    return Error::NoError;
}

QStringList DeviceCommander::argsFunc(Command command) const {
    switch (command) {
    case Command::SetParameter:
        return argsSetParameter();
    case Command::Date:
        return argsDate();
    case Command::ContentType:
        return argsContentType();
    case Command::UniqueId:
        return argsUniqueId();
    case Command::StatusString:
        return argsStatusString();
    case Command::VideoRecordMode:
        return argsVideoRecordMode();
    case Command::VideoDuration:
        return argsVideoDuration();
    case Command::ChargeDetectDelay:
        return argsChargeDetectDelay();
    case Command::LogWrite:
        return argsLogWrite();
    case Command::VideoRotation:
        return argsVideoRotation();
    case Command::FtpUsername:
        return argsFtpUsername();
    case Command::FtpPassword:
        return argsFtpPassword();
    case Command::Count:
        Q_ASSERT(false);
    }
    return {};
}

QStringList DeviceCommander::argsSetParameter() const {
    return { ip(), port(), rtspLogin(), rtspPassword() };
}

QStringList DeviceCommander::argsDate() const {
    return { QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss") };
}

QStringList DeviceCommander::argsContentType() const {
    return {};
}

QStringList DeviceCommander::argsUniqueId() const {
    return { uniqueId() };
}

QStringList DeviceCommander::argsStatusString() const {
    return { statusString() };
}

QStringList DeviceCommander::argsVideoRecordMode() const {
    return { videoRecordMode() };
}

QStringList DeviceCommander::argsVideoDuration() const {
    return { videoDuration() };
}

QStringList DeviceCommander::argsChargeDetectDelay() const {
    return { chargeDetectDelay() };
}

QStringList DeviceCommander::argsLogWrite() const {
    return { logWrite() };
}

QStringList DeviceCommander::argsVideoRotation() const {
    return { videoRotation() };
}

QStringList DeviceCommander::argsFtpUsername() const {
    return { ftpUsername() };
}

QStringList DeviceCommander::argsFtpPassword() const {
    return { ftpPassword() };
}

const QString &DeviceCommander::ip() const {
    return _ip;
}

void DeviceCommander::setIp(const QString &newIp) {
    if (_ip == newIp)
        return;
    _ip = newIp;
    emit ipChanged();
}

const QString &DeviceCommander::port() const {
    return _port;
}

void DeviceCommander::setPort(const QString &newPort) {
    if (_port == newPort)
        return;
    _port = newPort;
    emit portChanged();
}

const QString &DeviceCommander::rtspLogin() const {
    return _rtspLogin;
}

void DeviceCommander::setRtspLogin(const QString &newRtspLogin) {
    if (_rtspLogin == newRtspLogin)
        return;
    _rtspLogin = newRtspLogin;
    emit rtspLoginChanged();
}

const QString &DeviceCommander::rtspPassword() const {
    return _rtspPassword;
}

void DeviceCommander::setRtspPassword(const QString &newRtspPassword) {
    if (_rtspPassword == newRtspPassword)
        return;
    _rtspPassword = newRtspPassword;
    emit rtspPasswordChanged();
}

const QString &DeviceCommander::uniqueId() const {
    return _uniqueId;
}

void DeviceCommander::setUniqueId(const QString &newUniqueId) {
    if (_uniqueId == newUniqueId)
        return;
    _uniqueId = newUniqueId;
    emit uniqueIdChanged();
}

const QString &DeviceCommander::statusString() const {
    return _statusString;
}

void DeviceCommander::setStatusString(const QString &newStatusString) {
    if (_statusString == newStatusString)
        return;
    _statusString = newStatusString;
    emit statusStringChanged();
}

const QString &DeviceCommander::videoRecordMode() const {
    return _videoRecordMode;
}

void DeviceCommander::setVideoRecordMode(const QString &newVideoRecordMode) {
    if (_videoRecordMode == newVideoRecordMode)
        return;
    _videoRecordMode = newVideoRecordMode;
    emit videoRecordModeChanged();
}

const QString &DeviceCommander::videoDuration() const {
    return _videoDuration;
}

void DeviceCommander::setVideoDuration(const QString &newVideoDuration) {
    if (_videoDuration == newVideoDuration)
        return;
    _videoDuration = newVideoDuration;
    emit videoDurationChanged();
}

const QString &DeviceCommander::chargeDetectDelay() const {
    return _chargeDetectDelay;
}

void DeviceCommander::setChargeDetectDelay(const QString &newChargeDetectDelay) {
    if (_chargeDetectDelay == newChargeDetectDelay)
        return;
    _chargeDetectDelay = newChargeDetectDelay;
    emit chargeDetectDelayChanged();
}

const QString &DeviceCommander::logWrite() const {
    return _logWrite;
}

void DeviceCommander::setLogWrite(const QString &newLogWrite) {
    if (_logWrite == newLogWrite)
        return;
    _logWrite = newLogWrite;
    emit logWriteChanged();
}

const QString &DeviceCommander::videoRotation() const {
    return _videoRotation;
}

void DeviceCommander::setVideoRotation(const QString &newVideoRotation) {
    if (_videoRotation == newVideoRotation)
        return;
    _videoRotation = newVideoRotation;
    emit videoRotationChanged();
}

const QString &DeviceCommander::ftpUsername() const {
    return _ftpUsername;
}

void DeviceCommander::setFtpUsername(const QString &newFtpUsername) {
    if (_ftpUsername == newFtpUsername)
        return;
    _ftpUsername = newFtpUsername;
    emit ftpUsernameChanged();
}

const QString &DeviceCommander::ftpPassword() const {
    return _ftpPassword;
}

void DeviceCommander::setFtpPassword(const QString &newFtpPassword) {
    if (_ftpPassword == newFtpPassword)
        return;
    _ftpPassword = newFtpPassword;
    emit ftpPasswordChanged();
}

void DeviceCommander::socketReadyRead() {
    Q_ASSERT(_socket);
    qDebug() << __FILE__ << ":" << __LINE__
             << "DeviceCommander::socketReadyRead: " << _socket->readAll();
    setWaitForAnswer(false);
}

void DeviceCommander::socketStateChanged(QAbstractSocket::SocketState socketState) {
    auto socket = qobject_cast<QTcpSocket*>(sender());
    if (__timerId && socket == _socket
            && socketState != QAbstractSocket::ConnectingState
            && socketState != QAbstractSocket::HostLookupState) {
        qDebug() << __FILE__ << __LINE__ << socketState;
        killTimer(__timerId);
        __timerId = 0;
    }
    qDebug() << __FILE__ << ":" << __LINE__ << "StateChanged" << socket << socketState << _waitForAnswer;
    switch (socketState) {
    case QAbstractSocket::SocketState::ConnectedState:
        socketSendMsg();
        break;
    case QAbstractSocket::SocketState::UnconnectedState:
        if (socket == _socket) {
            setError(Error::CantConnecting);
            setWaitForAnswer(false);
        }
        break;
    default:
        break;
    }
}

void DeviceCommander::socketSendMsg() {
    Q_ASSERT(_socket);
    if (_sendMsg.isEmpty()) {
        qDebug() << __FILE__ << ":" << __LINE__ << "_sendMsg.isEmpty()";
        return;
    }
    if (_socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << __FILE__ << ":" << __LINE__ << "_socket->state() =" << _socket->state();
        return;
    }
    _socket->write(_sendMsg);
    qDebug() << __FILE__ << ":" << __LINE__ << "_socket->flush() = " << _socket->flush();
    _sendMsg.clear();
    setWaitForAnswer(true);
}

bool DeviceCommander::waitForAnswer() const {
    return _waitForAnswer;
}

void DeviceCommander::setWaitForAnswer(bool newWaitForAnswer) {
    if (_waitForAnswer == newWaitForAnswer)
        return;
    _waitForAnswer = newWaitForAnswer;
    emit waitForAnswerChanged();
}

DeviceCommander::Error DeviceCommander::error() const {
    return _error;
}

void DeviceCommander::setError(Error newError) {
    if (_error == newError)
        return;
    _error = newError;
    emit errorChanged();
}

void DeviceCommander::timerEvent(QTimerEvent *event) {
    if (event->timerId() == __timerId) {
        qDebug() << __FILE__ << __LINE__ << "timer! ! !";
        killTimer(__timerId);
        __timerId = 0;
        _socket->abort();
    } else {
        Q_ASSERT(false);
    }
}
