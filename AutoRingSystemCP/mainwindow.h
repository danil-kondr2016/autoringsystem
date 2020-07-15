/*

MainWindow.h

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDateTime>
#include <QDate>
#include <QTime>

#include <QtEvents>
#include <QEvent>
#include <QCloseEvent>

#include <QTextStream>
#include <QStandardItemModel>

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include <QTime>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include <QSettings>

#include "aboutdialog.h"
#include "settings.h"
#include "calculator.h"
#include "ringdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString lesson_file;
    bool file_is_changed;

    QString device_ip_address;
    bool time_sync;

    bool is_calculator = false;

    void closeEvent(QCloseEvent* event);

    void loadScheduleFromFile(QString filename);
    QByteArray requestPassword();

public Q_SLOTS:
    void newFile();

    void saveSchedule();
    void saveScheduleAs();
    void loadSchedule();
    void checkSchedule();

    void uploadSchedule();
    void downloadSchedule();
    void getScheduleFromCalculator(QStringList rows);

    void addLessonToEnd();
    void addLessonBefore();
    void addLessonAfter();
    void deleteLesson();

    void putTimeFromPC();
    void setTime();
    void updateSettings();

    void aboutApplication();

    void setChanged();
    void getResponse(QNetworkReply* rp);

    void invokeCalculator();

    void doRings();

    void tableViewContextMenu(QPoint position);

    void turnOnCalculatorMode();
    void turnOffCalculatorMode();

    void changeBreaksAndRecalculate(QStandardItem* item);
    void recalculateSchedule();

    void getPassword();

private:    
    Ui::MainWindow *ui;
    QStandardItemModel *schedule;
    AboutDialog *about;
    QNetworkAccessManager *network;
    Calculator *calc;

    QSettings *settings;
    Settings *setwindow;
};

#endif // MAINWINDOW_H
