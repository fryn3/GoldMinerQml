#pragma once

#include <QHash>

struct ConfigController
{
public:
    enum class Error {
        NoError,
        CantOpenFile,
        BadJsonFormat,
        BadParallelValue,
    };

    const QLatin1String CONFIG_FILE {"config.json"};
    const int DEFAULT_PARALLEL = 2;

    const QLatin1String KEY_AUTO_DONLOADING_PATH {"autoDownloadingPath"};
    const QLatin1String KEY_COUNT_PARALLEL {"countParallel"};
    const QLatin1String KEY_DEVICES {"devices"};
    const QLatin1String KEY_IP {"ip"};
    const QLatin1String KEY_MAC {"mac"};

    ConfigController() noexcept;
    ~ConfigController() noexcept;


    QString autoDownloadingPath;
    int countParallel = DEFAULT_PARALLEL;
    QHash<QString, QString> devices; // {mac, name}
    Error error() const noexcept;

    void setError(Error newError) noexcept;

private:
    Error _error = Error::NoError;
};

