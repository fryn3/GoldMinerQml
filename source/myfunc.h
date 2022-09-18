#pragma once

#include <QQmlEngine>

namespace My {

template<class C>
bool qmlRegisterUncreatableType(QString itemName) {
    qmlRegisterUncreatableType<C>(QString("cpp.%1").arg(itemName).toUtf8(),
                                  43, 21, itemName.toUtf8(),
                                  "Not creatable as it is an enum type");
    return true;
}


template<class C>
bool qmlRegisterType(QString itemName) {
    qmlRegisterType<C>(QString("cpp.%1").arg(itemName).toUtf8(),
                       43, 21, itemName.toUtf8());
    return true;
}

// Генерирует рандомное слово длиной len.
QString genRandom(const int len);

}   // namespace My
