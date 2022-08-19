#include "devicemodel.h"

#include <QJsonArray>

static const QString DEVICE_JSON_KEY = "Camera";
static const int FIELDS_COUNT = 11; // если добавятся поля, это число нужно обновить!


bool operator==(const DeviceModel::Device &l, const DeviceModel::Device &r) {
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
//            && l.ftpUsername == r.ftpUsername
//            && l.ftpPassword == r.ftpPassword
            && l.videoRotation == r.videoRotation
            && l.isSkip == r.isSkip;
}

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
    "isSkip"
};

DeviceModel::Device DeviceModel::Device::fromJson(QJsonObject jObject, QString mac, bool *ok)
{
    *ok = false;
    DeviceModel::Device d;
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

QJsonObject DeviceModel::Device::toJson(bool withoutMac) const
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

DeviceModel::DeviceModel(QObject *parent) : QAbstractListModel(parent) { }

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
            return _devices.at(index.row()).mac;
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
    case DmChargeDetectDalayRole:
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
    }
    return QVariant();
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()
            || index.row() >= _devices.count()
            || index.column() != 0) {
        return false;
    }

    switch (role) {
    case Qt::DisplayRole:
    case DmNameRole:
        _devices[index.row()].name = value.toString();
        return true;
    case DmMacRole:
        _devices[index.row()].mac = value.toString();
        return true;
    case DmIpRole:
        _devices[index.row()].ip = value.toString();
        return true;
    case DmOnameRole:
        _devices[index.row()].oName = value.toString();
        return true;
    case DmUniqueIdRole:
        _devices[index.row()].uniqueId = value.toString();
        return true;
    case DmStatusStringRole:
        _devices[index.row()].statusString = value.toString();
        return true;
    case DmModeRole:
        _devices[index.row()].mode = value.toInt();
        return true;
    case DmVideoDurationRole:
        _devices[index.row()].videoDuration = value.toInt();
        return true;
    case DmChargeDetectDalayRole:
        _devices[index.row()].chargeDetectDelay = value.toDouble();
        return true;
    case DmLogWriteRole:
        _devices[index.row()].logWrite = value.toBool();
        return true;
    case DmFtpUsernameRole:
        _devices[index.row()].ftpUsername = value.toString();
        return true;
    case DmFtpPasswordRole:
        _devices[index.row()].ftpPassword = value.toString();
        return true;
    case DmVideoRotationRole:
        _devices[index.row()].videoRotation = value.toBool();
        return true;
    case DmIsSkipRole:
        _devices[index.row()].isSkip = value.toBool();
        return true;
    }
    return false;
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
    QHash<int, QByteArray> r;
    for (int i = DmRoleBegin; i < DmRoleEnd; ++i) {
        r.insert(i, DM_ROLE_STR.at(i - DmRoleBegin).toUtf8());
    }
    return r;
}

void DeviceModel::addDevice(QString ip, QString mac, QString oName) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    Device d;
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

void DeviceModel::addDeviceFromIni(QString mac, DeviceModel::Device device) {
    Q_ASSERT(mac == device.mac);
    _savingDevices.insert(mac, device);
}

QList<DeviceModel::Device> DeviceModel::devices() const {
    return _devices;
}

void DeviceModel::clear() {
    beginResetModel();
    _devices.clear();
    endResetModel();
}
