#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <array>

class DeviceModel : public QAbstractListModel {
    Q_OBJECT
public:
    static const QString ITEM_NAME;     ///< DeviceModel
    static const bool IS_QML_REG;

    enum DevModelRole {
        DmRoleBegin = Qt::UserRole,

        DmNameRole = DmRoleBegin,
        DmMacRole,
        DmIpRole,
        DmOnameRole, // Имя на роутере
        DmUniqueIdRole,
        DmStatusStringRole,
        DmModeRole,
        DmVideoDurationRole,
        DmChargeDetectDalayRole, // fixme "delay"
        DmLogWriteRole,
        DmFtpUsernameRole,
        DmFtpPasswordRole,
        DmVideoRotationRole,
        DmIsSkipRole,

        DmRoleEnd
    };
    Q_ENUM(DevModelRole)
    static const int DM_ROLE_COUNT = DmRoleEnd - DmRoleBegin;
    static const std::array<QString, DM_ROLE_COUNT> DM_ROLE_STR;

    struct Device {
        friend bool operator==(const Device &l, const Device &r);
        static Device fromJson(QJsonObject jObject, QString mac = {}, bool *ok = nullptr);
        QJsonObject toJson(bool withoutMac = true) const;

        QString name;
        QString mac;
        QString ip;
        QString oName;
        QString uniqueId;
        QString statusString;
        int mode;
        int videoDuration = 60;
        double chargeDetectDelay = 0.5;
        bool logWrite = true;
        QString ftpUsername;
        QString ftpPassword;
        bool videoRotation = false;
        bool isSkip = false;
    };

    DeviceModel(QObject *parent = nullptr);
    virtual ~DeviceModel() = default;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariant get(int row, int role = Qt::DisplayRole) const;
    void addDevice(QString ip, QString mac = {}, QString oName = {});
    void addDeviceFromIni(QString mac, Device device);
    QList<Device> devices() const;
    void clear();
private:
    QList<Device> _devices; // {ip, mac} найденные девайсы
    QMap<QString, Device> _savingDevices; // { mac, name} из ini файла
};

