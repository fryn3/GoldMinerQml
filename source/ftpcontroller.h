#pragma once

#include <QFile>
#include <QObject>
#include "ftpmodel.h"

class FtpController : public QObject
{
    Q_OBJECT
public:

    enum class State {
        None,
        Downloading,
        Removing,
        Stoping
    };

    explicit FtpController(QObject *parent = nullptr);

    bool removeAfterDownload() const;
    void setRemoveAfterDownload(bool newRemoveAfterDownload);

    FtpModel *ftpModel() const;
    void setFtpModel(FtpModel *newFtpModel);

    const QString &deviceName() const;
    void setDeviceName(const QString &newDeviceName);

    State state() const;

public slots:
    void startDownloadAll(QString path);
    void removeAll();
    void stop();
signals:
    void started();
    void finished();
    void stoping();

private slots:
    void commandStartedSlot(int i);
    void commandFinishedSlot(int i, bool e);
    void ftpDone();
    void startNext();
private:
    void prepareProcess();
    void finish();
    void ftpConnect();
    void ftpDisconnect();

    State _state = State::None;
    FtpModel * _ftpModel = nullptr;
    QString _deviceName;
    bool _removeAfterDownload = false;
    bool _removeOnly = false;
    QString _path;
    int __indexDownload = -1;
    const QVector<FtpModel::RowStruct> * _files;
    QFile *_getFile = nullptr;
};

