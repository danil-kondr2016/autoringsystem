#include "constants.h"

QString C::confFileName()
{
    return QDir::homePath() + "/.autoringrc.ini";
}

const QString C::ADDRESS = "address";
const QString C::PASSWORD_HASH = "password_hash";
const QString C::PASSWORD_SALT = "password_salt";
const QString C::TIME_SYNC = "time_sync";
