#include "passwdtools.h"

#include <QCryptographicHash>
#include <random>
#include <ctime>

QByteArray generate_salt()
{
    char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*.";

    QByteArray result;

    std::mt19937 rnd(time(nullptr));

    for (int i = 0; i < 4; i++) {
        unsigned int N = rnd();
        result.append(b64[((N & 0x000000FFU)) & 0x3F]);
        result.append(b64[((N & 0x0000FF00U) >> 8)  & 0x3F]);
        result.append(b64[((N & 0x00FF0000U) >> 16) & 0x3F]);
        result.append(b64[((N & 0xFF000000U) >> 24) & 0x3F]);
    }

    return result;
}

QString get_password_hash(QByteArray password, QByteArray salt)
{
    QByteArray salted;
    salted.append(password);
    salted.append(salt);

    QByteArray hash;
    hash.append(salted);

    for (int i = 0; i < 1000; i++) {
        hash = QCryptographicHash::hash(salted, QCryptographicHash::Md5);
        salted.clear();
        salted.append(hash);
    }

    QString result;
    for (int i = 0; i < hash.length(); i++) {
        result += QString().sprintf("%02hhx", (unsigned char)hash[i]);
    }

    return result;
}
