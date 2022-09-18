#pragma once

#include <QObject>

#include "devicecommander.h"
#include "devicemodel.h"
#include "ftpmodel.h"

class DevWorker;

class DeviceController : public QObject {
    Q_OBJECT
public:
    explicit DeviceController(QObject *parent = nullptr);

    void setDeviceModel(DeviceModel *devModel);

public slots:
    void startDownload();

signals:

private:
    void initFtpServer(int indexDev);
    DeviceModel *_devModel;
    QHash<int, DevWorker> _devsCommander;
    int _countParallel = 2;
    int _currentDev = 0;
};

class DevWorker : public QObject {
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
public:
    enum class State {
        None,
        // Конфиг ФТП сервера. Ожидание ответа tcp.
        InitFtp,
        // Получение списка файлов.
        GetFtpList,

        CountState
    };
    DevWorker(QString ipStr, QString ftpLog, QString ftpPass, QObject *parent = nullptr);
    DevWorker(const DeviceCam& dev, QObject *parent = nullptr);
    DevWorker() = delete;
    DevWorker(const DevWorker&) = delete;
    DevWorker& operator=(const DevWorker&) = delete;
    ~DevWorker();


    const QString ip;
    const QString ftpUsername;
    const QString ftpPassword;
    const QString mode = "1"; /// \todo пусть тут будет всегда 1. Уточнить у Жени.

    State state() const;

signals:
    void stateChanged();

private slots:
    void stateMachine();
private:
    void setState(State newState);

    State _state = State::None;
    DeviceCommander _commander;
    FtpModel _ftpModel;

};
