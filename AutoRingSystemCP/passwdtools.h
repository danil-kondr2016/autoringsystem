#ifndef PASSWDTOOLS_H
#define PASSWDTOOLS_H

#include <QByteArray>
#include <QString>

QByteArray generate_salt();
QString get_password_hash(QByteArray password, QByteArray salt);

#endif // PASSWDTOOLS_H
