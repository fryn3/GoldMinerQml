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
public:
    enum class State {
        None,
        Downloding,
        Finished,

        Count
    };
    Q_ENUM(State)
    explicit DeviceController(QObject *parent = nullptr);
    DeviceController(const DeviceController&) = delete;
    DeviceController& operator=(const DeviceController&) = delete;

    void setDeviceModel(DeviceModel *devModel);

    const QString &downloadFolder() const;
    void setDownloadFolder(const QString &newDownloadFolder);

public slots:
    void startDownloading();

signals:
    void started();
    void finished();
    void downloadFolderChanged();

private slots:
    void workerStarting();
private:
    void configFtpServer(int indexDev);
    DeviceModel *_devModel;
    QString _downloadFolder;
    QHash<int, DevWorker*> _devWorkers;
    int _countParallel = 2;
    int _currentDev = 0;
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
    Q_PROPERTY(int progressTotal READ progressTotal NOTIFY progressTotalChanged FINAL)
    Q_PROPERTY(int progressDone READ progressDone NOTIFY progressDoneChanged FINAL)
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
    DevWorker(int index, QString ipStr, QString ftpLog, QString ftpPass, QString folderPath, QString name, QObject *parent = nullptr);
    DevWorker(int index, const DeviceCam& dev, QString folderPath, QString name, QObject *parent = nullptr);
    DevWorker(const DevWorker&) = delete;
    DevWorker& operator=(const DevWorker&) = delete;

    const int indexRow;
    const QString ip;
    const QString ftpUsername;
    const QString ftpPassword;
    const QString mode = "1"; /// \todo пусть тут будет всегда 1. Уточнить у Жени.
    const QString downloadFolder;
    const QString dirName;  // deviceName || Unique ID || MAC

    State state() const;

    int progressTotal() const;
    int progressDone() const;

public slots:
    void startDownloading();

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
    void setProgressTotal(int newProgressTotal);
    void setProgressDone(int newProgressDone);

    State _state = State::None;
    DeviceCommander _commander;
    FtpModel _ftpModel;
    QFile _file;

    // Размер всех элементов в ftp.
    int _progressTotal = 1;
    // Сколько уже скачано.
    int _progressDone = 0;
    // Для текущего файла предыдущее значение прогресса.
    int __progressPrevDoneInFile = 0;

    QList<QUrlInfo> _filesToDownload;
};
