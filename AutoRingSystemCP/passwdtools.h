#ifndef PASSWDTOOLS_H
#define PASSWDTOOLS_H

#include <QByteArray>
#include <QString>
#include <QWidget>

#include "constants.h"

enum PasswordCheckState {
    PASS_CANCEL = -1,
    PASS_INCORRECT,
    PASS_CORRECT
};

QByteArray generate_salt();
QString get_password_hash(QByteArray password, QByteArray salt);

QString request_password(QWidget * parent);
PasswordCheckState check_password();

#endif // PASSWDTOOLS_H
