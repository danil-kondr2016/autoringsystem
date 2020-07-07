/*

Settings.cpp

Copyright (C) 2019 Danila Kondratenko <dan.kondratenko2013@ya.ru>

    This file is part of Autoring System.

    Autoring System is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл - часть Системы автоподачи звонков.

   Система автоподачи звонков - свободная программа: вы можете перераспространять ее и/или изменять
   ее на условиях Стандартной общественной лицензии GNU в том виде, в каком
   она была опубликована Фондом свободного программного обеспечения; либо
   версии 3 лицензии, либо (по вашему выбору) любой более поздней версии.

   Эта программа распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)


*/

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
        settings->setValue("sync-time", true);
    } else {
        settings->setValue("sync-time", false);
    }

    settings->setValue("password_hash", password_hash);
    settings->setValue("password_salt", QString(password_salt));

    hide();
    emit applied();
}

void Settings::changePassword()
{
    bool typed;

    QString old_passwd;
    if (password_hash != QString() && password_salt != QByteArray()) {
        old_passwd = QInputDialog::getText(
                    this, "Изменение пароля", "Введите старый пароль: ",
                    QLineEdit::Password, QString(), &typed
                    );
        if (!typed) return;
    } else {
        old_passwd = "";
    }

    QString old_passwd_hash = get_password_hash(old_passwd.toUtf8(), password_salt);
    if ((old_passwd_hash != password_hash) && (password_hash != QByteArray() && password_salt != QByteArray())) {
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
