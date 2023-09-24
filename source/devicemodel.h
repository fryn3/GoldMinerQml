#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <array>

struct DeviceCam;

class DeviceModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged FINAL)
public:
    static const QString ITEM_NAME;     ///< DeviceModel
    static const bool IS_QML_REG;

    enum DevModelRole {
        DmRoleBegin = Qt::UserRole + 1,

        DmNameRole = DmRoleBegin,
        DmMacRole,
        DmIpRole,
        DmOnameRole, // Имя на роутере
        DmFtpUsernameRole,
        DmFtpPasswordRole,
        DmIsSkipRole,

        DmStructRole,   // Возвращает структуру DeviceCam
        DmTotalSizeRole,
        DmDoneSizeRole,

        DmRoleEnd
    };
    Q_ENUM(DevModelRole)
    static const int DM_ROLE_COUNT = DmRoleEnd - DmRoleBegin;
    static const std::array<QString, DM_ROLE_COUNT> DM_ROLE_STR;

    DeviceModel(QObject *parent = nullptr);
    virtual ~DeviceModel() = default;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QVariant get(int row, int role = Qt::DisplayRole) const;
    Q_INVOKABLE bool set(int row, const QVariant &value, int role = Qt::DisplayRole);
    void addDevice(QString ip, QString mac = {}, QString oName = {}, QString name = {});
    bool setName(QString mac, QString name);
    void parseSettingsIni(const QString &settingsIni, int row);
    const QVector<DeviceCam> &devices() const;
    const QHash<QString, QString> &macAndName() const;
    void setMacAndName(const QHash<QString, QString> &newMacAndName);
    void clear();
    int findRow(QString ip) const;
    void clearExceptFor(QList<QString> ips);
    void removeRow(QString ip);
    void setProgressBarZero();

public slots:
    void addDeviceSlot(QString ip, QString mac, QString oName);
signals:
    void rowCountChanged();
private:
    void setMacAndName(QString mac, QString name);
    QVector<DeviceCam> _devices; // найденные девайсы
    QHash<QString, QString> _macAndName; // {mac and name}
};

struct DeviceCam {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString mac MEMBER mac)
    Q_PROPERTY(QString ip MEMBER ip)
    Q_PROPERTY(QString oName MEMBER oName)
    Q_PROPERTY(QString ftpUsername MEMBER ftpUsername)
    Q_PROPERTY(QString ftpPassword MEMBER ftpPassword)
    Q_PROPERTY(bool isSkip MEMBER isSkip)
    Q_PROPERTY(qint64 totalSize MEMBER totalSize)
    Q_PROPERTY(qint64 doneSize MEMBER doneSize)

public:
    friend bool operator==(const DeviceCam &l, const DeviceCam &r);
    friend bool operator!=(const DeviceCam &l, const DeviceCam &r);
    static DeviceCam fromJson(QJsonObject jObject, QString mac = {}, bool *ok = nullptr);
    QJsonObject toJson(bool withoutMac = true) const;
    QString name;
    QString mac;
    QString ip;
    QString oName;
    QString ftpUsername;
    QString ftpPassword;
    bool isSkip = false;
    qint64 totalSize = 1;
    qint64 doneSize = 0;
};

Q_DECLARE_METATYPE(DeviceCam)
