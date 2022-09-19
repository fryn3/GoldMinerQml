#include "devicemodel.h"
#include "myfunc.h"
#include "qcoreapplication.h"

#include <QJsonArray>

static const QString DEVICE_JSON_KEY = "Camera";
static const int FIELDS_COUNT = 11; // если добавятся поля, это число нужно обновить!

bool operator==(const DeviceCam &l, const DeviceCam &r) {
    return l.name == r.name
            && l.mac == r.mac
            && l.ip == r.ip
            && l.oName == r.oName
            && l.uniqueId == r.uniqueId
            && l.statusString == r.statusString
            && l.mode == r.mode
            && l.videoDuration == r.videoDuration
            && l.chargeDetectDelay == r.chargeDetectDelay
            && l.logWrite == r.logWrite
            && l.ftpUsername == r.ftpUsername
            && l.ftpPassword == r.ftpPassword
            && l.videoRotation == r.videoRotation
            && l.isSkip == r.isSkip;
}

bool operator!= (const DeviceCam &l, const DeviceCam &r) {
    return !(l == r);
}

const QString DeviceModel::ITEM_NAME = "DeviceModel";
static void regT() {
    My::qmlRegisterType<DeviceModel>(DeviceModel::ITEM_NAME);
}
Q_COREAPP_STARTUP_FUNCTION(regT)
const bool DeviceModel::IS_QML_REG = true;//My::qmlRegisterType<DeviceModel>(DeviceModel::ITEM_NAME);


const std::array<QString, DeviceModel::DM_ROLE_COUNT>
        DeviceModel::DM_ROLE_STR {
    "name",
    "mac",
    "ip",
    "oName",
    "uniqueId",
    "statusString",
    "mode",
    "videoDuration",
    "chargeDetectDelay",
    "logWrite",
    "ftpUsername",
    "ftpPassword",
    "videoRotation",
    "isSkip",
    "struct"
};

DeviceCam DeviceCam::fromJson(QJsonObject jObject, QString mac, bool *ok)
{
    *ok = false;
    DeviceCam d;
    if (!jObject.contains(DEVICE_JSON_KEY)) {
        return d;
    }
    auto jValue = jObject.value(DEVICE_JSON_KEY);
    if (!jValue.isArray()) {
        return d;
    }
    QJsonArray jA = jValue.toArray();
    if (jA.size() != FIELDS_COUNT - !mac.isEmpty()) {
        return d;
    }
    d.name = jA[0].toString();
    if (mac.isEmpty()) {
        d.mac = jA[1].toString();
    } else {
        d.mac = mac;
    }
    d.oName = jA[2 - !mac.isEmpty()].toString();
    d.uniqueId = jA[3 - !mac.isEmpty()].toString();
    d.statusString = jA[4 - !mac.isEmpty()].toString();
    d.mode = jA[5 - !mac.isEmpty()].toInt();
    d.videoDuration = jA[6 - !mac.isEmpty()].toInt();
    d.chargeDetectDelay = jA[7 - !mac.isEmpty()].toDouble();
    d.logWrite = jA[8 - !mac.isEmpty()].toBool();
    d.videoRotation = jA[9 - !mac.isEmpty()].toBool();
    d.isSkip = jA[10 - !mac.isEmpty()].toBool();
    *ok = true;
    return d;
}

QJsonObject DeviceCam::toJson(bool withoutMac) const
{
    QJsonArray jA;
    jA << name;
    if (!withoutMac) {
        jA << withoutMac;
    }
    jA << oName;
    jA << uniqueId;
    jA << statusString;
    jA << mode;
    jA << videoDuration;
    jA << chargeDetectDelay;
    jA << logWrite;
    jA << videoRotation;
    jA << isSkip;
    QJsonObject jObject;
    jObject[DEVICE_JSON_KEY] = jA;
    return jObject;
}

DeviceModel::DeviceModel(QObject *parent) : QAbstractListModel(parent) {
    connect(this, &QAbstractItemModel::rowsInserted, this, &DeviceModel::rowCountChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &DeviceModel::rowCountChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &DeviceModel::rowCountChanged);
}

int DeviceModel::rowCount(const QModelIndex &/*parent*/) const {
    return _devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= _devices.count()
            || index.column() != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        if (_devices.at(index.row()).name.isEmpty()) {
            if (_devices.at(index.row()).uniqueId.isEmpty()) {
                return _devices.at(index.row()).mac;
            } else {
                return _devices.at(index.row()).uniqueId;
            }
        } else {
            return _devices.at(index.row()).name;
        }
    case DmNameRole:
        return _devices.at(index.row()).name;
    case DmMacRole:
        return _devices.at(index.row()).mac;
    case DmIpRole:
        return _devices.at(index.row()).ip;
    case DmOnameRole:
        return _devices.at(index.row()).oName;
    case DmUniqueIdRole:
        return _devices.at(index.row()).uniqueId;
    case DmStatusStringRole:
        return _devices.at(index.row()).statusString;
    case DmModeRole:
        return _devices.at(index.row()).mode;
    case DmVideoDurationRole:
        return _devices.at(index.row()).videoDuration;
    case DmChargeDetectDelayRole:
        return _devices.at(index.row()).chargeDetectDelay;
    case DmLogWriteRole:
        return _devices.at(index.row()).logWrite;
    case DmFtpUsernameRole:
        return _devices.at(index.row()).ftpUsername;
    case DmFtpPasswordRole:
        return _devices.at(index.row()).ftpPassword;
    case DmVideoRotationRole:
        return _devices.at(index.row()).videoRotation;
    case DmIsSkipRole:
        return _devices.at(index.row()).isSkip;
    case DmStructRole:
        return QVariant::fromValue(_devices.at(index.row()));
    case DmTotalSizeRole:
        return _devices.at(index.row()).totalSize;
    case DmDoneSizeRole:
        return _devices.at(index.row()).doneSize;
    }
    return QVariant();
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()
            || index.row() >= _devices.count()
            || index.column() != 0) {
        qDebug() << __FILE__ << __LINE__ << "invalid" << index << value << role;
        return false;
    }

    if (role != Qt::DisplayRole
            && !(role >= DmRoleBegin && role < DmRoleEnd)) {
        qDebug() << __FILE__ << __LINE__ << "bad role" << index << value << role;
        return false;
    }

    switch (role) {
    case Qt::DisplayRole:
    case DmNameRole:
        _devices[index.row()].name = value.toString();
        break;
    case DmMacRole:
        _devices[index.row()].mac = value.toString();
        break;
    case DmIpRole:
        _devices[index.row()].ip = value.toString();
        break;
    case DmOnameRole:
        _devices[index.row()].oName = value.toString();
        break;
    case DmUniqueIdRole:
        _devices[index.row()].uniqueId = value.toString();
        break;
    case DmStatusStringRole:
        _devices[index.row()].statusString = value.toString();
        break;
    case DmModeRole:
        _devices[index.row()].mode = value.toInt();
        break;
    case DmVideoDurationRole:
        _devices[index.row()].videoDuration = value.toInt();
        break;
    case DmChargeDetectDelayRole:
        _devices[index.row()].chargeDetectDelay = value.toDouble();
        break;
    case DmLogWriteRole:
        _devices[index.row()].logWrite = value.toBool();
        break;
    case DmFtpUsernameRole:
        _devices[index.row()].ftpUsername = value.toString();
        break;
    case DmFtpPasswordRole:
        _devices[index.row()].ftpPassword = value.toString();
        break;
    case DmVideoRotationRole:
        _devices[index.row()].videoRotation = value.toBool();
        break;
    case DmIsSkipRole:
        _devices[index.row()].isSkip = value.toBool();
        break;
    case DmStructRole:
        _devices[index.row()] = value.value<DeviceCam>();
        break;
    case DmTotalSizeRole:
        _devices[index.row()].totalSize = value.toInt();
        break;
    case DmDoneSizeRole:
        _devices[index.row()].doneSize = value.toInt();
        break;
    }
    if (role == Qt::DisplayRole
            || role == DmNameRole
            || role == DmMacRole
            || role == DmUniqueIdRole) {
        emit dataChanged(index, index, { Qt::DisplayRole, role, DmStructRole });
    } else if (role == DmStructRole) {
        emit dataChanged(index, index);
    } else {
        emit dataChanged(index, index, { role, DmStructRole });
    }
    return true;
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
    QHash<int, QByteArray> r;
    for (int i = DmRoleBegin; i < DmRoleEnd; ++i) {
        r.insert(i, DM_ROLE_STR.at(i - DmRoleBegin).toUtf8());
    }
    return r;
}

QVariant DeviceModel::get(int row, int role) const {
    return data(index(row), role);
}

bool DeviceModel::set(int row, const QVariant &value, int role) {
    bool r = setData(index(row), value, role);
    qDebug() << __FILE__ << __LINE__ << row << value << role << " ==> " << r;
    return r;
}

void DeviceModel::addDevice(QString ip, QString mac, QString oName) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    DeviceCam d;
    if (!mac.isEmpty()) {
        if (_savingDevices.contains(mac)) {
            d = _savingDevices.value(mac);
        } else {
            d.name = mac;
        }
    } else {
        d.name = ip;
    }
    d.mac = mac;
    d.ip = ip;
    d.oName = oName;
    if (!_devices.contains(d)) {
        _devices.append(d);
    }
    endInsertRows();
}

void DeviceModel::addDeviceFromIni(QString mac, DeviceCam device) {
    Q_ASSERT(mac == device.mac);
    _savingDevices.insert(mac, device);
}

void DeviceModel::parseSettingsIni(const QString &settingsIni, int row) {
    qDebug() << __LINE__ << settingsIni;
//    const auto WLAN_SSID = QLatin1String("WLAN_SSID");  // не нужно!
//    const auto WLAN_PWD = QLatin1String("WLAN_PWD");    // не нужно!
//    const auto RTSP_USER = QLatin1String("RTSP_USER");  // не нужно!
//    const auto RTSP_PWD = QLatin1String("RTSP_PWD");    // не нужно!
    const auto UNIQUE_ID = QLatin1String("UNIQUE_ID");
    const auto STATUS_STRING = QLatin1String("STATUS_STRING");
    const auto CHARGE_DETECT_DELAY = QLatin1String("CHARGE_DETECT_DELAY");
    const auto VIDEO_DURATION = QLatin1String("VIDEO_DURATION");
    const auto VIDEO_RECORD_MODE = QLatin1String("VIDEO_RECORD_MODE");
//    const auto BUTTON_PRESS_DURATION = QLatin1String("BUTTON_PRESS_DURATION"); // не нужно!
//    const auto SETTINGS_UPD = QLatin1String("SETTINGS_UPD"); // не нужно!
    const auto LOG_WRITE = QLatin1String("LOG_WRITE");
    const auto VIDEO_ROTATION = QLatin1String("VIDEO_ROTATION");

    const QMap<QLatin1String, int> m {
        {UNIQUE_ID, DmUniqueIdRole},
        {STATUS_STRING, DmStatusStringRole},
        {CHARGE_DETECT_DELAY, DmChargeDetectDelayRole},
        {VIDEO_DURATION, DmVideoDurationRole},
        {VIDEO_RECORD_MODE, DmModeRole},
        {LOG_WRITE, DmLogWriteRole},
        {VIDEO_ROTATION, DmVideoRotationRole},
    };

    auto settingsRows = settingsIni.split("\n", Qt::SkipEmptyParts);

    for (const auto& rowStr: settingsRows) {
        if (rowStr.startsWith("#")) {
            continue;
        }
        auto value = rowStr.section("\"", 1, 1);

        for (auto it = m.constBegin(); it != m.constEnd(); ++it) {
            if (rowStr.startsWith(it.key())) {
                setData(index(row), value, it.value());
                break;
            }
        }
    }
}

QList<DeviceCam> DeviceModel::devices() const {
    return _devices;
}

void DeviceModel::clear() {
    beginResetModel();
    _devices.clear();
    endResetModel();
}
