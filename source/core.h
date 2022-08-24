#pragma once

#include <QObject>
#include "devicemodel.h"
#include "ftpmodel.h"

class Core : public QObject {
    Q_OBJECT
    Q_PROPERTY(DeviceModel * deviceModel READ devModel CONSTANT FINAL)
    Q_PROPERTY(int devModelCurrentIndex READ devModelCurrentIndex WRITE setDevModelCurrentIndex RESET resetDevModelCurrentIndex NOTIFY devModelCurrentIndexChanged FINAL)
    Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
public:
    static const QString ITEM_NAME;     ///< Core
    static const bool IS_QML_REG;

    enum class State {
        None,
        FindingDevices,

        CountState
    };
    Q_ENUM(State)

    explicit Core(QObject *parent = nullptr);

    DeviceModel * const devModel();

    State state() const;
    void setState(State newState);

    int devModelCurrentIndex() const;
    void setDevModelCurrentIndex(int newDevModelCurrentIndex);
    void resetDevModelCurrentIndex();

public slots:
    void findDevices();
signals:

    /// \brief Показывать сообщение в строке состояния.
    /// \param timeoutMilisec - если отрицательное число, то не скрывать сообщение.
    void showMessage(QString msg, int timeoutMilisec = 0);

    void stateChanged();

    void devModelCurrentIndexChanged();

private:
    DeviceModel _devModel;
    int _devModelCurrentIndex = -1;
    FtpModel _ftpModel;
    State _state;
};

