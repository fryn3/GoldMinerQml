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
        DmUniqueIdRole,
        DmStatusStringRole,
        DmModeRole,
        DmVideoDurationRole,
        DmChargeDetectDelayRole,
        DmLogWriteRole,
        DmFtpUsernameRole,
        DmFtpPasswordRole,
        DmVideoRotationRole,
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
    QVector<DeviceCam> devices() const;
    QHash<QString, QString> macAndName() const;
    void clear();

signals:
    void rowCountChanged();
private:
    QVector<DeviceCam> _devices; // найденные девайсы
};

struct DeviceCam {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString mac MEMBER mac)
    Q_PROPERTY(QString ip MEMBER ip)
    Q_PROPERTY(QString oName MEMBER oName)
    Q_PROPERTY(QString uniqueId MEMBER uniqueId)
    Q_PROPERTY(QString statusString MEMBER statusString)
    Q_PROPERTY(int mode MEMBER mode)
    Q_PROPERTY(int videoDuration MEMBER videoDuration)
    Q_PROPERTY(double chargeDetectDelay MEMBER chargeDetectDelay)
    Q_PROPERTY(bool logWrite MEMBER logWrite)
    Q_PROPERTY(QString ftpUsername MEMBER ftpUsername)
    Q_PROPERTY(QString ftpPassword MEMBER ftpPassword)
    Q_PROPERTY(bool videoRotation MEMBER videoRotation)
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
    QString uniqueId;
    QString statusString;
    int mode = 1;
    int videoDuration = 60;
    double chargeDetectDelay = 0.5;
    bool logWrite = true;
    QString ftpUsername;
    QString ftpPassword;
    bool videoRotation = false;
    bool isSkip = false;
    qint64 totalSize = 1;
    qint64 doneSize = 0;
};

Q_DECLARE_METATYPE(DeviceCam)
