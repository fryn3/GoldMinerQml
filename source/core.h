#pragma once

#include <QFile>
#include <QObject>
#include "devicecommander.h"
#include "devicecontroller.h"
#include "devicemodel.h"
#include "ftpmodel.h"

class Core : public QObject {
    Q_OBJECT
    Q_PROPERTY(DeviceModel * deviceModel READ devModel CONSTANT FINAL)
    Q_PROPERTY(FtpModel * ftpModel READ ftpModel CONSTANT FINAL)
    Q_PROPERTY(DeviceController * deviceController READ devController CONSTANT FINAL)
    Q_PROPERTY(int devModelCurrentIndex READ devModelCurrentIndex WRITE setDevModelCurrentIndex RESET resetDevModelCurrentIndex NOTIFY devModelCurrentIndexChanged FINAL)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(DeviceCam currentDeviceCam READ currentDeviceCam WRITE setCurrentDeviceCam RESET resetCurrentDeviceCam NOTIFY currentDeviceCamChanged FINAL)
public:
    static const QString ITEM_NAME;     ///< Core
    static const bool IS_QML_REG;

    enum class State {
        None,

        // При нажатии на кнопку "Поиск устройств".
        FindingDevices,

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

        CountState
    };
    Q_ENUM(State)

    explicit Core(QObject *parent = nullptr);

    DeviceModel * devModel();

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

public slots:
    void findDev();
    void showFtpFiles();
    void readDevConfig();
    void writeDevConfig();
    void updateCurrentDeviceCam();

    // Обработка изменений состояний.
    // В теории должен отслеживать любые изменения.
    void stateMachine();

signals:

    /// \brief Показывать сообщение в строке состояния.
    /// \param timeoutMilisec - если отрицательное число, то не скрывать сообщение.
    void showMessage(QString msg, int timeoutMilisec = 0);

    void stateChanged();

    void devModelCurrentIndexChanged();

    void currentDeviceCamChanged();

private:
    void initFtpServer();

    DeviceModel _devModel;
    int _devModelCurrentIndex = -1;
    DeviceCam _currentDeviceCam;
    DeviceCommander _devCommander;
    FtpModel _ftpModel;
    QByteArray _settArray;
    State _state;
    QTemporaryFile *_settingsFile = nullptr;

    DeviceController _devController;
    // Нужно подключить констроллер к кнопке,
    // инициализировать переменные,
    // законнектить сигнал finished().
};
