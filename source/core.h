#pragma once

#include <QObject>
#include "devicemodel.h"
#include "ftpmodel.h"

class Core : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* deviceModel READ devModel CONSTANT FINAL)
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

public slots:
    void findDevices();
signals:
    /// \brief Показывать сообщение в строке состояния.
    /// \param timeoutMilisec - если отрицательное число, то не скрывать сообщение.
    void showMessage(QString msg, int timeoutMilisec = 0);

    void stateChanged();

private:
    DeviceModel _devModel;
    FtpModel _ftpModel;
    State _state;
};

