#pragma once

#include <QFile>
#include <QObject>
#include "configcontroller.h"
#include "devicecommander.h"
#include "devicecontroller.h"
#include "devicemodel.h"
#include "ftpcontroller.h"
#include "ftpmodel.h"

class Core : public QObject {
    Q_OBJECT
    Q_PROPERTY(DeviceModel * deviceModel READ devModel CONSTANT FINAL)
    Q_PROPERTY(int devicesFound READ devicesFound NOTIFY devicesFoundChanged FINAL)
    Q_PROPERTY(FtpModel * ftpModel READ ftpModel NOTIFY ftpModelChanged FINAL)
    Q_PROPERTY(DeviceController * deviceController READ devController CONSTANT FINAL)
    Q_PROPERTY(QString deviceControllerPath READ deviceControllerPath NOTIFY deviceControllerPathChanged FINAL)
    Q_PROPERTY(int devModelCurrentIndex READ devModelCurrentIndex WRITE setDevModelCurrentIndex RESET resetDevModelCurrentIndex NOTIFY devModelCurrentIndexChanged FINAL)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(DeviceCam currentDeviceCam READ currentDeviceCam WRITE setCurrentDeviceCam RESET resetCurrentDeviceCam NOTIFY currentDeviceCamChanged FINAL)
    Q_PROPERTY(bool someDebugFlag READ someDebugFlag WRITE setSomeDebugFlag NOTIFY someDebugFlagChanged)
public:
    static const QString ITEM_NAME;     ///< Core
    static const bool IS_QML_REG;

    enum class State {
        None,

        // При нажатии на кнопку "Поиск устройств".
        FindingDevices,

        // В процессе автоскачивания.
        ProcessAutoDownloading,
        // В процессе остановки автоскачивания.
        StoppingAutoDownloading,

        // Конфиг ФТП сервера. Ожидание ответа tcp.
        ShowFtpFilesInitFtp,
        // Получение списка файлов.
        ShowFtpFilesGetList,

        // Конфиг ФТП сервера. Ожидание ответа tcp.
        ReadingConfigInitFtp,

        // Получаю список корневых файлов.
        // Дальше буду скачивать и парсить файл settings.ini.
        ReadingConfigWaitFtp,

        // Скачиваю settings.ini и паршу.
        ReadingConfigDownloading,

        // Записываю данные на камеру.
        WriteingConfigWaitTcp,

        ProcessDownloading,
        ProcessRemoving,
        ProcessStoping,

        CountState
    };
    Q_ENUM(State)

    explicit Core(QObject *parent = nullptr);
    virtual ~Core() noexcept;

    DeviceModel * devModel();
    int devicesFound() const;

    State state() const;
    void setState(State newState);

    int devModelCurrentIndex() const;
    void setDevModelCurrentIndex(int newDevModelCurrentIndex);
    void resetDevModelCurrentIndex();

    DeviceCam currentDeviceCam() const;
    void setCurrentDeviceCam(DeviceCam newCurrentDeviceCam);
    void resetCurrentDeviceCam();

    FtpModel * ftpModel();

    DeviceController *devController();
    QString deviceControllerPath() const;

    bool someDebugFlag() const;
    void setSomeDebugFlag(bool newSomeDebugFlag);

public slots:
    void findDev();
    void runAutoDownloading();
    void stopAutoDownloading();
    void showFtpFiles();
    void readDevConfig();
    void writeDevConfig();
    void updateCurrentDeviceCam();
    void cleanDevice();
    void stopCleanDevice();
    void downloadDevice(QString pathDir, bool removeAfterDownlod);
    void stopDownloadDevice();

signals:

    /// \brief Показывать сообщение в строке состояния.
    /// \param timeoutMilisec - если отрицательное число, то не скрывать сообщение.
    void showMessage(QString msg, int timeoutMilisec = 0);

    void stateChanged();

    void devicesFoundChanged();

    void deviceControllerPathChanged();

    void devModelCurrentIndexChanged();

    void currentDeviceCamChanged();

    void ftpModelChanged();

    void someDebugFlagChanged();

private slots:
    void ftpReconnect();

private:
    // Обработка изменений состояний.
    // В теории должен отслеживать любые изменения.
    void stateMachine();

    void initFtpServer();

    DeviceModel _devModel;
    int _devModelCurrentIndex = -1;
    DeviceCam _currentDeviceCam;
    DeviceCommander _devCommander;
    FtpModel *_ftpModel = nullptr;
    QByteArray _settArray;
    State _state = State::None;
    QTemporaryFile *_settingsFile = nullptr;

    DeviceController _devController;
    ConfigController _config;
    FtpController _ftpController;



    // for debug
    bool _someDebugFlag = false;
};
