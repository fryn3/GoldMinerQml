#pragma once

#include "devicemodel.h"

#include <QObject>
#include <QTcpSocket>

class DeviceCommander : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool waitForAnswer READ waitForAnswer NOTIFY waitForAnswerChanged FINAL)
    Q_PROPERTY(QString ip READ ip WRITE setIp NOTIFY ipChanged FINAL)
    Q_PROPERTY(QString port READ port WRITE setPort NOTIFY portChanged FINAL)
    Q_PROPERTY(QString rtspLogin READ rtspLogin WRITE setRtspLogin NOTIFY rtspLoginChanged FINAL)
    Q_PROPERTY(QString rtspPassword READ rtspPassword WRITE setRtspPassword NOTIFY rtspPasswordChanged FINAL)
    Q_PROPERTY(QString uniqueId READ uniqueId WRITE setUniqueId NOTIFY uniqueIdChanged FINAL)
    Q_PROPERTY(QString statusString READ statusString WRITE setStatusString NOTIFY statusStringChanged FINAL)
    Q_PROPERTY(QString videoRecordMode READ videoRecordMode WRITE setVideoRecordMode NOTIFY videoRecordModeChanged FINAL)
    Q_PROPERTY(QString videoDuration READ videoDuration WRITE setVideoDuration NOTIFY videoDurationChanged FINAL)
    Q_PROPERTY(QString chargeDetectDelay READ chargeDetectDelay WRITE setChargeDetectDelay NOTIFY chargeDetectDelayChanged FINAL)
    Q_PROPERTY(QString logWrite READ logWrite WRITE setLogWrite NOTIFY logWriteChanged FINAL)
    Q_PROPERTY(QString videoRotation READ videoRotation WRITE setVideoRotation NOTIFY videoRotationChanged FINAL)
    Q_PROPERTY(QString ftpUsername READ ftpUsername WRITE setFtpUsername NOTIFY ftpUsernameChanged FINAL)
    Q_PROPERTY(QString ftpPassword READ ftpPassword WRITE setFtpPassword NOTIFY ftpPasswordChanged FINAL)
    Q_PROPERTY(Error error READ error WRITE setError NOTIFY errorChanged)
public:
    static const QString ITEM_NAME;     ///< DeviceCommander
    static const bool IS_QML_REG;

    enum class Command {
        SetParameter,
        Date,
        ContentType,
        UniqueId,
        StatusString,
        VideoRecordMode,
        VideoDuration,
        ChargeDetectDelay,
        LogWrite,
        VideoRotation,
        FtpUsername,
        FtpPassword,
        Count
    };
    Q_ENUM(Command)

    enum class Error {
        NoError,

        // Пропущен какой аргумент у какой-то команды.
        MissArgument,

        BadArgument,
        // Отправляем команду недождавшись ответа.
        WaitForAnswer,
        // Не смог подключиться к хосту.
        CantConnecting,

        Count
    };
    Q_ENUM(Error)

    static const QMap<Command, QString> COMMAND_PATTERN;

    explicit DeviceCommander(QObject *parent = nullptr);

    void setData(DeviceCam d);
    // Если массив пустой, отправляются все команды.
    Error sendCommands(const QSet<Command> &commands = {});

    bool waitForAnswer() const;

    QStringList argsFunc(Command command) const;
    QStringList argsSetParameter() const;
    QStringList argsDate() const;
    QStringList argsContentType() const;
    QStringList argsUniqueId() const;
    QStringList argsStatusString() const;
    QStringList argsVideoRecordMode() const;
    QStringList argsVideoDuration() const;
    QStringList argsChargeDetectDelay() const;
    QStringList argsLogWrite() const;
    QStringList argsVideoRotation() const;
    QStringList argsFtpUsername() const;
    QStringList argsFtpPassword() const;


    const QString &ip() const;
    void setIp(const QString &newIp);

    const QString &port() const;
    void setPort(const QString &newPort);

    const QString &rtspLogin() const;
    void setRtspLogin(const QString &newRtspLogin);

    const QString &rtspPassword() const;
    void setRtspPassword(const QString &newRtspPassword);

    const QString &uniqueId() const;
    void setUniqueId(const QString &newUniqueId);

    const QString &statusString() const;
    void setStatusString(const QString &newStatusString);

    const QString &videoRecordMode() const;
    void setVideoRecordMode(const QString &newVideoRecordMode);

    const QString &videoDuration() const;
    void setVideoDuration(const QString &newVideoDuration);

    const QString &chargeDetectDelay() const;
    void setChargeDetectDelay(const QString &newChargeDetectDelay);

    const QString &logWrite() const;
    void setLogWrite(const QString &newLogWrite);

    const QString &videoRotation() const;
    void setVideoRotation(const QString &newVideoRotation);

    const QString &ftpUsername() const;
    void setFtpUsername(const QString &newFtpUsername);

    const QString &ftpPassword() const;
    void setFtpPassword(const QString &newFtpPassword);

    Error error() const;
    void setError(Error newError);

signals:
    void waitForAnswerChanged();

    void ipChanged();
    void portChanged();
    void rtspLoginChanged();
    void rtspPasswordChanged();
    void uniqueIdChanged();
    void statusStringChanged();
    void videoRecordModeChanged();
    void videoDurationChanged();
    void chargeDetectDelayChanged();
    void logWriteChanged();
    void videoRotationChanged();
    void ftpUsernameChanged();
    void ftpPasswordChanged();

    void errorChanged();

protected:
    void timerEvent(QTimerEvent *event) override;
private slots:
    void socketReadyRead();
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void socketSendMsg();
private:
    void setWaitForAnswer(bool newWaitForAnswer);

    QTcpSocket *_socket = nullptr;
    bool _waitForAnswer = false;
    QByteArray _sendMsg;
    QString _ip;
    QString _port = "3056";
    QString _rtspLogin = "admin";
    QString _rtspPassword = "12345678";
    QString _uniqueId;
    QString _statusString;
    QString _videoRecordMode;
    QString _videoDuration;
    QString _chargeDetectDelay;
    QString _logWrite;
    QString _videoRotation;
    QString _ftpUsername;
    QString _ftpPassword;
    Error _error = Error::NoError;
    int _timeOut = 3000; // Время для сторожевого таймера
    int __timerId = 0;

};

inline uint qHash(const DeviceCommander::Command& c) { return qHash(int(c)); }
