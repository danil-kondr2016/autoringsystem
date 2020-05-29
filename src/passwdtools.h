#ifndef PASSWDTOOLS_H
#define PASSWDTOOLS_H

#include <QByteArray>

QByteArray generate_salt();
QByteArray get_password_hash(QByteArray password, QByteArray salt);

#endif // PASSWDTOOLS_H
