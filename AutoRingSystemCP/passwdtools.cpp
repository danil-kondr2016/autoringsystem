#include "passwdtools.h"

#include <QCryptographicHash>
#include <QSettings>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>

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
    char hex[] = "0123456789abcdef";

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
        result.append(hex[(hash[i] >> 4) & 0x0F]);
        result.append(hex[hash[i] & 0x0F]);
    }

    return result;
}

QString request_password(QWidget * parent = nullptr)
{
    QSettings settings(C::confFileName(), QSettings::IniFormat);

    QString password_hash = settings.value(C::PASSWORD_HASH).toString();
    QByteArray password_salt = settings.value(C::PASSWORD_SALT).toByteArray();

    if (password_hash.isEmpty() && password_salt.isEmpty())
        return QString("");

    QString password;

    bool typed;
    password = QInputDialog::getText(
                 parent, "Пароль", "Введите пароль: ",
                 QLineEdit::Password, QString(), &typed
               );

    if (!typed)
        return QString();

    return get_password_hash(password.toUtf8(), password_salt);
}

PasswordCheckState check_password()
{
    QSettings settings(C::confFileName(), QSettings::IniFormat);

    QString password_hash = settings.value(C::PASSWORD_HASH).toString();
    if (password_hash.isEmpty())
        return PASS_CORRECT;

    QString hash = request_password();

    if (hash.isNull())
        return PASS_CANCEL;
    else
        return (hash == password_hash) ? PASS_CORRECT : PASS_INCORRECT;
}
