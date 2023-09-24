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
        FatalErrorFindIp,
        // Плохой путь к папке загрузки.
        FatalErrorBadDir
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

    int countParallel() const;
    void setCountParallel(int newCountParallel);

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
    DeviceModel *_devModel = nullptr;
    QString _downloadFolder;
    QHash<QString, DevWorker*> _devWorkers; // key is IP
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
    Q_PROPERTY(Error error READ error NOTIFY errorChanged FINAL)
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

    enum class Error {
        None,
        BadDir,
        FtpSomeError,
    };
    Q_ENUM(Error)

    DevWorker(QString ipStr, QString ftpLog, QString ftpPass, QString folderPath, QStringList subDirsList, QObject *parent = nullptr);
    DevWorker(const DeviceCam& dev, QString folderPath, QStringList subDirs, QObject *parent = nullptr);
    virtual ~DevWorker() noexcept;
    DevWorker(const DevWorker&) = delete;
    DevWorker& operator=(const DevWorker&) = delete;

    const QString ip;
    const QString ftpUsername;
    const QString ftpPassword;
    const QString downloadFolder;
    const QStringList subDirs;  // deviceName -> time

    State state() const;

    qint64 progressTotal() const;
    qint64 progressDone() const;

    Error error() const;

    bool removeAfterDownload() const;
    void setRemoveAfterDownload(bool newRemoveAfterDownload);

    bool onlyRemove() const;
    void setOnlyRemove(bool newOnlyRemove);

public slots:
    void startDownloading();
    void stopDownloading();

signals:
    void stateChanged();

    void started();
    void finished();

    void progressTotalChanged();
    void progressDoneChanged();

    void errorChanged();

private slots:
    void stateMachine();
private:
    void setState(State newState);
    void setProgressTotal(qint64 newProgressTotal);
    void setProgressDone(qint64 newProgressDone);
    void setError(Error newError);

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

    QList<FtpModel::RowStruct> _filesToDownload;
    bool _stoped = false;
    Error _error = Error::None;

    bool _removeAfterDownload = true;
    bool _onlyRemove = false;

};
