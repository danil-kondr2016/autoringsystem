#include "settings.h"
#include "ui_settings.h"

#include "passwdtools.h"

#include <QDir>
#include <QMessageBox>

#include <QDebug>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings),
    settings(new QSettings(QDir::homePath() + QString("/.autoringrc.ini"), QSettings::IniFormat))
{
    ui->setupUi(this);
    connect(this, &Settings::accepted, this, &Settings::accept);

    device_ip_address = settings->value("ip-address").toString();
    time_sync = settings->value("time-sync").toBool();
    password_hash = settings->value("password_hash").toByteArray();
    password_salt = settings->value("password_salt").toByteArray();

    ui->ip_address->setText(device_ip_address);
    if (time_sync) {
        ui->sync_time->setCheckState(Qt::Checked);
    } else {
        ui->sync_time->setCheckState(Qt::Unchecked);
    }
}

void Settings::accept()
{
    int is_sync = ui->sync_time->checkState();
    QString ip_address = ui->ip_address->text();
    settings->clear();
    settings->setValue("ip-address", ip_address);
    if (is_sync == Qt::Checked) {
        settings->setValue("time-sync", true);
    } else {
        settings->setValue("time-sync", false);
    }

    settings->setValue("password_hash", password_hash);
    settings->setValue("password_salt", QString(password_salt));

    emit applied();
    close();
}

void Settings::changePassword()
{
    bool typed;

    QString old_passwd;
    if (!password_hash.isEmpty() && !password_salt.isEmpty()) {
        old_passwd = QInputDialog::getText(
                    this, "Изменение пароля", "Введите старый пароль: ",
                    QLineEdit::Password, QString(), &typed
                    );
        if (!typed) return;
    } else {
        old_passwd = "";
    }

    QString old_passwd_hash = get_password_hash(old_passwd.toUtf8(), password_salt);
    if ((old_passwd_hash != password_hash) && (!password_hash.isEmpty() && !password_salt.isEmpty())) {
        QMessageBox::critical(this, "Ошибка", "Неверный пароль");
        return;
    } else {
        QString new_password, new_password2;
        new_password = QInputDialog::getText(
                         this, "Изменение пароля", "Введите новый пароль: ",
                         QLineEdit::Password, QString(), &typed
                       );
        if (!typed) return;
        new_password2 = QInputDialog::getText(
                          this, "Изменение пароля", "Введите пароль ещё раз: ",
                          QLineEdit::Password, QString(), &typed
                        );
        if (!typed) return;

        if (new_password != new_password2) {
            QMessageBox::critical(
                this, "Ошибка", "Пароли не совпадают"
            );
            return;
        } else {
            old_password_hash = password_hash;
            password_salt = generate_salt();
            password_hash = get_password_hash(new_password.toUtf8(), password_salt);
        }
    }
}

Settings::~Settings()
{
    delete settings;
    delete ui;
}
