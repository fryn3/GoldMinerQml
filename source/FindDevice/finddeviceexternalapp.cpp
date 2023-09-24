#include "finddeviceexternalapp.h"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTemporaryFile>

int FindDeviceExternalApp::counterFindDev = 0;

FindDeviceExternalApp::FindDeviceExternalApp(bool isDebugMode, QObject *parent)
        : FindDeviceControllerBase{parent}, DEBUG_MODE(isDebugMode) {
    ++counterFindDev;
}

void FindDeviceExternalApp::start() {
    QFile findIpExe(":/resources/soft/FindIP.exe");
    QTemporaryFile *newTempFile = QTemporaryFile::createNativeFile(findIpExe);
    newTempFile->rename(newTempFile->fileName() + ".exe");
    qDebug() << "File name is" << newTempFile->fileName();
    QProcess *p = new QProcess(this);
    connect(p, &QProcess::errorOccurred, this, [this, newTempFile] {
        setError(QString("Ошибка запуска %1").arg(newTempFile->fileName()));
        sender()->deleteLater();
        newTempFile->remove();
        newTempFile->deleteLater();
        emit finished();
    });

    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            p, &QProcess::deleteLater);
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [newTempFile] {
        newTempFile->remove();
        newTempFile->deleteLater();
    });
    connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FindDeviceControllerBase::finished);
    connect(p, &QProcess::readyReadStandardOutput, this, [this] {
        auto p = dynamic_cast<QProcess*>(sender());
        QString s = p->readAllStandardOutput();
        auto rows = s.split("\r\n", Qt::SkipEmptyParts);
        for (auto &row: rows) {
            auto words = row.split(" ");
            qDebug() << words;
            auto ip = words.at(1);
            auto mac = words.at(3);
            auto oName = words.at(5);
            if (firstSuitableNames.contains(oName)) {
                emit findedDevice(ip, mac, oName);
                if (DEBUG_MODE && counterFindDev % 2 == 1) {
                    break;
                }
            }
        }
    });
    p->start(newTempFile->fileName(), {"VideoServer"});
    emit started();
}
