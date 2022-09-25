#pragma once

#include <memory>

#include <QFile>
#include <QObject>

#include "devicecommander.h"
#include "devicemodel.h"
#include "ftpmodel.h"

class DevWorker;

class DeviceController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString downloadFolder READ downloadFolder WRITE setDownloadFolder NOTIFY downloadFolderChanged)
    Q_PROPERTY(bool stoped READ stoped WRITE setStoped NOTIFY stopedChanged)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
public:
    enum class State {
        None,
        // Поиск устройств.
        FindingDevices,
        // Скачивание файлов.
        Downloding,
        // Ожидание перед повторным запуском.
        Wait,
        // Ошибка (при запуске FindIP.exe).
        FatalError
    };
    Q_ENUM(State)
    explicit DeviceController(QObject *parent = nullptr);
    DeviceController(const DeviceController&) = delete;
    DeviceController& operator=(const DeviceController&) = delete;

    void setDeviceModel(DeviceModel *devModel);

    const QString &downloadFolder() const;
    void setDownloadFolder(const QString &newDownloadFolder);

    bool stoped() const;
    void setStoped(bool newStoped);

    State state() const;
    void setState(State newState);

    int waitTimeMs() const;
    void setWaitTimeMs(int newWaitTimeMs);

public slots:
    void startDownloading();
    void stopDownloading();

signals:
    void started();
    void finished();

    void downloadFolderChanged();
    void stopedChanged();
    void stateChanged();

protected:
    void timerEvent(QTimerEvent *event) override;
private slots:
    void workerStarting();
private:
    void findDev();
    void configFtpServer(int indexDev);
    DeviceModel *_devModel;
    QString _downloadFolder;
    QHash<int, DevWorker*> _devWorkers;
    int _countParallel = 2; /// \todo Показать Жене. Количество параллельных скачиваний
    int _currentDev = 0;
    bool _stoped = false;
    State _state = State::None;
    int _waitTimeMs = 10000; /// Ожидание перед повторным запуском
    int __timerIdWait = 0;
};

//!
//! \brief Класс для работы с одной камерой
//!
//! Подключается к камере, скачивает и удаляет файлы.
//! Для запуска скачивания, необходима вызвать startDownloading().
//! По завершении вызывает сигнал finished().
//!
class DevWorker : public QObject {
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
    Q_PROPERTY(qint64 progressTotal READ progressTotal NOTIFY progressTotalChanged FINAL)
    Q_PROPERTY(qint64 progressDone READ progressDone NOTIFY progressDoneChanged FINAL)
public:
    enum class State {
        None,
        // Конфиг ФТП сервера. Ожидание ответа tcp.
        InitFtp,
        // Получение списка файлов.
        GetFtpList,
        // Поочередное скачивание файлов.
        GetFtpFiles,
        // Все файлы должны быть скачены и удалены
        AllFilesDownloded,

        CountState
    };
    Q_ENUM(State)
    DevWorker(int index, QString ipStr, QString ftpLog, QString ftpPass, QString folderPath, QStringList subDirsList, QObject *parent = nullptr);
    DevWorker(int index, const DeviceCam& dev, QString folderPath, QStringList subDirs, QObject *parent = nullptr);
    virtual ~DevWorker() noexcept;
    DevWorker(const DevWorker&) = delete;
    DevWorker& operator=(const DevWorker&) = delete;

    const int indexRow;
    const QString ip;
    const QString ftpUsername;
    const QString ftpPassword;
    const QString downloadFolder;
    const QStringList subDirs;  // deviceName -> time

    State state() const;

    qint64 progressTotal() const;
    qint64 progressDone() const;

public slots:
    void startDownloading();
    void stopDownloading();

signals:
    void stateChanged();

    void started();
    void finished();

    void progressTotalChanged();
    void progressDoneChanged();

private slots:
    void stateMachine();
private:
    void setState(State newState);
    void setProgressTotal(qint64 newProgressTotal);
    void setProgressDone(qint64 newProgressDone);

    State _state = State::None;
    DeviceCommander _commander;
    FtpModel _ftpModel;
    QFile _file;

    // Размер всех элементов в ftp.
    qint64 _progressTotal = 1;
    // Сколько уже скачано.
    qint64 _progressDone = 0;
    // Для текущего файла предыдущее значение прогресса.
    qint64 __progressPrevDoneInFile = 0;

    QList<QUrlInfo> _filesToDownload;
    bool _stoped = false;
};
