#include "myfunc.h"

#include <QRandomGenerator>

// Устанавливает простой лог/пароль
#define SIMPLE_PASS

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
QString My::genRandom(const int len) {
#ifdef SIMPLE_PASS
    static bool log = false;
    log = !log;
    return log ? "logQwerty" : "passQwerty";
#else
    static const QString alphanum =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    QString tmp(len, Qt::Uninitialized);

    for (int i = 0; i < len; ++i) {
        tmp[i] = alphanum.at(QRandomGenerator::global()->bounded(alphanum.length() - 1));
    }

    return tmp;
#endif
}
