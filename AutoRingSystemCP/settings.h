#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QtNetwork>

#include "constants.h"

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

    QString device_address;
    bool time_sync;

    QString old_password_hash, password_hash;
    QByteArray password_salt;

Q_SIGNALS:
    void applied();

public Q_SLOTS:
    void accept();

    void changePassword();

private:
    Ui::Settings *ui;
    QSettings *settings;
};

#endif // SETTINGS_H
