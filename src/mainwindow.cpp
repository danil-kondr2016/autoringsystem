/*

MainWindow.cpp

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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>

#include <QException>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    schedule(new QStandardItemModel),
    about(new AboutDialog),
    network(new QNetworkAccessManager),
    calc(new Calculator),
    settings(new QSettings("configuration.ini", QSettings::IniFormat)),
    setwindow(new Settings)

{
    ui->setupUi(this);

    schedule->setColumnCount(3);
    schedule->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");
    schedule->setRowCount(1);

    ui->tableView->setModel(schedule);

    lesson_file = QString("");
    file_is_changed = false;

    connect(schedule, &QStandardItemModel::dataChanged, this, &MainWindow::setChanged);
    connect(ui->action_settings, &QAction::triggered, setwindow, &Settings::show);
    connect(setwindow, &Settings::applied, this, &MainWindow::updateSettings);
    connect(network, &QNetworkAccessManager::finished, this, &MainWindow::getResponse);
    connect(calc, &Calculator::scheduleMade, this, &MainWindow::getScheduleFromCalculator);

    device_ip_address = settings->value("ip-address").toString();
    time_sync = settings->value("time-sync").toBool();

    if (time_sync) putTimeFromPC();
}

void MainWindow::putTimeFromPC()
{
    QDateTime curtime = QDateTime::currentDateTime();
    QNetworkRequest req;

    QString curtime_string = "time=";
    curtime_string += curtime.toString("HH:mm:ss").replace(":", "%3A");

    QString url = "http://" + device_ip_address + "/autoring";
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray data;

    data.append("method=set&");
    data.append(curtime_string);

    network->post(req, data);
}

void MainWindow::updateSettings() {
    device_ip_address = settings->value("ip-address").toString();
    time_sync = settings->value("time-sync").toBool();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    int answer;
    if (file_is_changed) {
        answer = QMessageBox::warning(this, "Система автоматической подачи звонков", "Сохранить расписание?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (answer != QMessageBox::Cancel) {
            if (answer == QMessageBox::Yes) {
                saveSchedule();
            }
            calc->close();
            setwindow->close();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        calc->close();
        setwindow->close();
        event->accept();
    }
}

void MainWindow::setChanged()
{
    file_is_changed = true;
}

void MainWindow::checkSchedule()
{
    int rowCnt = schedule->rowCount();
    int columnCnt = schedule->columnCount();

    int error = 0;
    int error_num = 0;

    QList<QModelIndex> row;
    QTime st, et;
    QTime ost, oet;

    for (int rn = 0; (rn < rowCnt) && (error == 0); rn++) {
        for (int cn = 0; cn < columnCnt; cn++) {
            row.append(schedule->index(rn, cn));
        }

        if ((row[0].data().toString() == "") || (row[1].data().toString() == "")) {
            error = 1;
            error_num = rn + 1;
        }

        st = row[0].data().toTime();
        et = row[1].data().toTime();

        if (st > et) {
            if (error == 0) {
                error = -1;
                error_num = rn + 1;
            }
        }
        if ((ost > et) || (oet > st)) {
            if (error == 0) {
                error = -2;
            }
        }
        ost = st;
        oet = et;
        row.clear();
    }

    if (error == -2) {
        ui->statusBar->showMessage("В расписании нарушен порядок уроков");
    } else if (error == -1) {
        ui->statusBar->showMessage(
                    QString().sprintf(
                        "Урок %d: конец урока раньше его начала", error_num
                        ));
    } else if (error == 0) {
        ui->statusBar->showMessage("Расписание составлено правильно");
    } else if (error == 1) {
        ui->statusBar->showMessage(
                    QString().sprintf(
                        "Урок %d: отсутствуют временные границы", error_num
                        ));
    }
}

void MainWindow::loadSchedule()
{
    QString filename = QFileDialog::getOpenFileName(this, QString(), QString(), "*.sch");
    if (filename == "") {
        return;
    } else {
        ui->tableView->setModel(nullptr);
        lesson_file = filename;
    }

    schedule->clear();
    schedule->setColumnCount(3);
    schedule->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");

    QFile file(lesson_file);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", QString("Файл по адресу %0 не обнаружен").arg(lesson_file));
    } else {
        QList<QStandardItem*> row;
        QTextStream fin(&file);
        QString tmp;
        while (!fin.atEnd()) {
            tmp = fin.readLine();
            QStringList xa = tmp.split(",");
            for (int i = 1; i < xa.length(); i++) {
                row.append(new QStandardItem(xa[i]));
            }

            schedule->appendRow(row);
            row.clear();
        }
        file.close();
    }
    ui->tableView->setModel(schedule);
    file_is_changed = false;
}

void MainWindow::loadScheduleFromFile(QString filename)
{
    ui->tableView->setModel(nullptr);
    lesson_file = filename;

    schedule->clear();
    schedule->setColumnCount(3);
    schedule->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");

    QFile file(lesson_file);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", QString("Файл по адресу %0 не обнаружен").arg(lesson_file));
    } else {
        QList<QStandardItem*> row;
        QTextStream fin(&file);
        QString tmp;
        while (!fin.atEnd()) {
            tmp = fin.readLine();
            QStringList xa = tmp.split(",");
            for (int i = 1; i < xa.length(); i++) {
                row.append(new QStandardItem(xa[i]));
            }

            schedule->appendRow(row);
            row.clear();
        }
        file.close();
    }
    ui->tableView->setModel(schedule);
    file_is_changed = false;
}

void MainWindow::getScheduleFromCalculator(QStringList rows)
{
    ui->tableView->setModel(nullptr);

    schedule->clear();
    schedule->setColumnCount(3);
    schedule->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");

    for (QString x : rows) {
        QStringList row;
        for (QString y : x.split(",")) row.append(y);

        QList<QStandardItem*> schedRow;
        schedRow.append(new QStandardItem(row[1]));
        schedRow.append(new QStandardItem(row[2]));
        schedRow.append(new QStandardItem(row[3]));

        schedule->appendRow(schedRow);
        schedRow.clear();
    }

    ui->tableView->setModel(schedule);
    ui->statusBar->showMessage("Расписание уроков рассчитано");
}

void MainWindow::saveSchedule()
{
    ui->statusBar->clearMessage();
    if (!QFile::exists(lesson_file) || lesson_file == "") {
        QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), "*.sch");
        if (filename == "") {
            ui->tableView->setModel(schedule);
            return;
        } else {
            ui->tableView->setModel(nullptr);
            lesson_file = filename;
        }
    }

    QFile file(lesson_file);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (lesson_file != "") {
            QMessageBox::critical(this, "Ошибка", QString("Файл по адресу %0 не удалось открыть или создать").arg(lesson_file));
        }
    } else {
        QTextStream fout(&file);
        QString towrite, item;
        int rc = schedule->rowCount();
        int cc = schedule->columnCount();
        QModelIndex index;
        for (int row = 0; row < rc; row++) {
            towrite += QString::number(row+1);
            for (int column = 0; column < cc; column++) {
                towrite += ",";
                index = schedule->index(row, column);
                item = index.data().toString();
                if (item == "") {
                    item = "0";
                }
                towrite += item;
            }
            towrite += "\n";
        }
        fout << towrite;
        file.close();
    }
    ui->tableView->setModel(schedule);
    file_is_changed = false;
}

void MainWindow::saveScheduleAs()
{
    ui->statusBar->clearMessage();
    QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), "*.sch");
    if (filename == "") {
        return;
    } else {
        ui->tableView->setModel(nullptr);
        lesson_file = filename;
    }

    QFile file(lesson_file);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (lesson_file != "") {
            QMessageBox::critical(this, "Ошибка", QString("Файл по адресу %0 не удалось открыть или создать").arg(lesson_file));
        } else {
            QMessageBox::critical(this, "Ошибка", QString("Файл не был выбран").arg(lesson_file));
        }
    } else {
        QTextStream fout(&file);
        QString towrite, item;
        int rc = schedule->rowCount();
        int cc = schedule->columnCount();
        QModelIndex index;
        for (int row = 0; row < rc; row++) {
            towrite += QString::number(row+1);
            for (int column = 0; column < cc; column++) {
                towrite += ",";
                index = schedule->index(row, column);
                item = index.data().toString();
                if (item == "") {
                    item = "0";
                }
                towrite += item;
            }
            towrite += "\n";
        }
        fout << towrite;
        file.close();
    }
    ui->tableView->setModel(schedule);
    file_is_changed = false;
}

void MainWindow::newFile()
{
    int answer;
    if (file_is_changed) {
        answer = QMessageBox::warning(this, "Система автоматической подачи звонков", "Сохранить расписание?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (answer != QMessageBox::Cancel) {
            if (answer == QMessageBox::Yes) {
                saveSchedule();
            }
        } else {
            return;
        }
    }

    ui->tableView->setModel(nullptr);

    schedule->clear();
    schedule->setRowCount(1);
    schedule->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");

    ui->tableView->setModel(schedule);
}

void MainWindow::addLessonToEnd()
{
    int rc = schedule->rowCount();
    if ((rc + 1) > 15) {
        ui->statusBar->showMessage("В расписании не может быть более 15 уроков");
        return;
    } else {
        ui->statusBar->clearMessage();
        QList<QStandardItem*> newRow;
        for (int i = 0; i < 3; i++)
            newRow.append(new QStandardItem);
        schedule->appendRow(newRow);
        newRow.clear();
    }
}

void MainWindow::addLessonBefore()
{
    int rc = schedule->rowCount();
    int current_row = ui->tableView->selectionModel()->currentIndex().row();
    if ((rc + 1) > 15) {
        ui->statusBar->showMessage("В расписании не может быть более 15 уроков");
        return;
    } else {
        ui->statusBar->clearMessage();
        QList<QStandardItem*> newRow;
        for (int i = 0; i < 3; i++)
            newRow.append(new QStandardItem);
        schedule->insertRow(current_row, newRow);
        newRow.clear();
    }
}

void MainWindow::addLessonAfter()
{
    int rc = schedule->rowCount();
    int current_row = ui->tableView->selectionModel()->currentIndex().row();
    if ((rc + 1) > 15) {
        ui->statusBar->showMessage("В расписании не может быть более 15 уроков");
        return;
    } else {
        ui->statusBar->clearMessage();
        QList<QStandardItem*> newRow;
        for (int i = 0; i < 3; i++)
            newRow.append(new QStandardItem);
        schedule->insertRow(current_row+1, newRow);
        newRow.clear();
    }
}

void MainWindow::deleteLesson()
{
    int rc = schedule->rowCount();
    int current_row = ui->tableView->selectionModel()->currentIndex().row();
    if ((rc - 1) <= 0) {
        ui->statusBar->clearMessage();
        schedule->removeRow(current_row);
        schedule->setRowCount(1);
    } else {
        ui->statusBar->clearMessage();
        schedule->removeRow(current_row);
    }
}

void MainWindow::aboutApplication()
{
    about->show();
}

void MainWindow::uploadSchedule()
{
    QNetworkRequest req;

    QString url = "http://" + device_ip_address + "/autoring";
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Connection", "Keep-Alive");

    QByteArray data;

    data.append("method=set&");
    QString x;

    QModelIndex index;

    int rn = schedule->rowCount();
    int cn = schedule->columnCount();

    x += QString().sprintf("lessnum=%d&schedule=", rn);

    for (int i = 0; i < rn; i++) {
        x += QString().sprintf("%d-", i+1);

        for (int j = 0; j < cn; j++) {
            index = schedule->index(i, j);
            if (index.data().toTime() != QTime()) {
                QTime z = index.data().toTime();
                x += QString().sprintf("%d.", z.hour()*60 + z.minute());
            } else {
                x += index.data().toString();
            }
        }
        x += "_";
    }

    data.append(x);

    network->post(req, data);
}

void MainWindow::downloadSchedule()
{
    QNetworkRequest req;

    QString url = "http://" + device_ip_address + "/autoring";
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray data;

    data.append("method=schedule");

    QByteArray answer;
    QList<QStandardItem*> row;

    network->post(req, data);
    file_is_changed = true;
}

void MainWindow::setTime()
{
    bool typed;
    QString time_string = QInputDialog::getText(this, "Ввод времени", "Введите время в виде \"чч:мм:сс\"", QLineEdit::Normal, QString(), &typed);
    if (!typed) return;
    QTime time = QTime::fromString(time_string, "HH:mm:ss");

    if (time.toString() == "") return;

    QString curtime_string = "time=";
    curtime_string += time.toString("HH:mm:ss").replace(":", "%3A");
 
    QNetworkRequest req;

    QString url = "http://" + device_ip_address + "/autoring";
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray data;

    data.append("method=set&");
    data.append(curtime_string);

    network->post(req, data);
}

void MainWindow::getResponse(QNetworkReply* rp)
{
    QByteArray answer;
    QList<QStandardItem*> row;

    if (rp->error() == QNetworkReply::NoError) {
        answer = rp->readAll();

        for (QByteArray x : QByteArray(answer).split('&')) {
            QStringList sx;
            for (QByteArray y : x.split('=')) sx.append(QByteArray::fromPercentEncoding(y));
            if (sx[0] == "schedule") {
                ui->tableView->setModel(nullptr);

                schedule->clear();
                schedule->setColumnCount(3);
                schedule->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");

                QString record;
                for (QChar x : sx[1]) {
                    if (x != '_') record += x;
                    else {
                        int ln, ls, le, rn;
                        sscanf(record.toUtf8().data(), "%d-%d.%d.%d", &ln, &ls, &le, &rn);
                        int lsh, lsm, leh, lem;
                        lsh = ls/60; lsm = ls % 60;
                        leh = le/60; lem = le % 60;

                        row.append(new QStandardItem(QString().sprintf("%02d:%02d", lsh, lsm)));
                        row.append(new QStandardItem(QString().sprintf("%02d:%02d", leh, lem)));
                        row.append(new QStandardItem(QString::number(rn)));

                        schedule->appendRow(row);
                        row.clear();
                        record.clear();
                    }
                }
                ui->tableView->setModel(schedule);
                ui->statusBar->showMessage("Расписание получено успешно");
            } else if (sx[0] == "state") {
                if (sx[1] == "0") ui->statusBar->showMessage("Операция выполнена успешно");
            }
        }
    } else {
        QMessageBox::critical(this, "Ошибка", QString("Произошла ошибка при обращении к устройству по адресу %0.").arg(device_ip_address));
    }
}

void MainWindow::invokeCalculator()
{
    calc->show();
}

void MainWindow::doRings()
{
    int ring_number, ring_delay, ring_pause;

    RingDialog* ringDialog = new RingDialog(this);
    ringDialog->initialize(1, 3);

    if (ringDialog->exec() == QDialog::Accepted) {
        ring_number = ringDialog->quantity;
        ring_delay = ringDialog->time;
        ring_pause = ringDialog->pause;
        delete ringDialog;
    } else {
        delete ringDialog;
        return;
    }

    QNetworkRequest req;

    QString url = "http://" + device_ip_address + "/autoring";
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray data;

    data.append("method=doring&");
    data.append(QString().sprintf("number=%d&", ring_number));
    data.append(QString().sprintf("time=%d&", ring_delay));
    if (ring_pause >= 1) {
        data.append(QString().sprintf("pause=%d", ring_pause));
    }

    network->post(req, data);
}

void MainWindow::tableViewContextMenu(QPoint position)
{
    QMenu *menu = new QMenu(this);

    QAction *action_addBefore = new QAction("Добавить урок перед выделенным", menu);
    action_addBefore->setIcon(QIcon(":/images/add_before.png"));

    QAction *action_addAfter = new QAction("Добавить урок после выделенного", menu);
    action_addAfter->setIcon(QIcon(":/images/add_after.png"));

    QAction *action_deleteLesson = new QAction("Удалить выбранный урок", menu);
    action_deleteLesson->setIcon(QIcon(":/images/delete.png"));

    connect(action_addBefore, SIGNAL(triggered()), this, SLOT(addLessonBefore()));
    connect(action_addAfter, SIGNAL(triggered()), this, SLOT(addLessonAfter()));
    connect(action_deleteLesson, SIGNAL(triggered()), this, SLOT(deleteLesson()));

    menu->addAction(action_addBefore);
    menu->addAction(action_addAfter);
    menu->addAction(action_deleteLesson);

    menu->popup(ui->tableView->viewport()->mapToGlobal(position));
}

MainWindow::~MainWindow()
{
    delete schedule;
    delete network;
    delete settings;

    delete calc;
    delete about;
    delete setwindow;
    delete ui;
}
