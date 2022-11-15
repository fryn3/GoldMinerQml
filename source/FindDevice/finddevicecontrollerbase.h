#pragma once

#include <QObject>

class FindDeviceControllerBase : public QObject {
    Q_OBJECT
public:

    /// Реализован фабричный метод
    ///
    /// Для одинакого источника во всей программы, рекомендуется вызывать
    /// этот статичный метод. Метод вернет указатель на объект
    /// FindDeviceSimulation or FindDeviceExternalApp в нужной конфигурации.
    static FindDeviceControllerBase* createController(QObject *parent);

    explicit FindDeviceControllerBase(QObject *parent = nullptr);

    virtual void start() = 0;
    bool isError();
    const QString &errorMsg() const;

signals:
    void started();
    void finished();
    void findedDevice(QString ip, QString mac, QString name = "lwip0");
    void error();
protected:
    /// Устанавливает _error = true, сохраняет errorMsg и вызывает сигнал error()
    void setError(const QString &newErrorMsg = {});
private:
    QString _errorMsg;
    bool _error = false;
};

