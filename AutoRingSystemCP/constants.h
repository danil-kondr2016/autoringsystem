#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QDir>

#define MAX_LESSONS 15

class C {
public:
    static QString confFileName();

    static const QString CONF_FILE_NAME;
    static const QString ADDRESS;
    static const QString PASSWORD_HASH;
    static const QString PASSWORD_SALT;
    static const QString TIME_SYNC;
};

#endif // CONSTANTS_H
