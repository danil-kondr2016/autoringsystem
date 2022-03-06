#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "passwdtools.h"
#include "scheduletools.h"

#include <stdio.h>
#include <cstdarg>
#include <algorithm>

#include <QDebug>
#include <QDir>
#include <QException>

int qstring_to_minutes(QString str)
{
    QTime time = QTime::fromString(str, "H:mm");
    return time.hour() * 60 + time.minute();
}

QString minutes_to_qstring(int minutes) {
    int h = minutes / 60;
    int m = minutes % 60;

    return QTime(h, m).toString("HH:mm");
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    schedule(new QStandardItemModel),
    about(new AboutDialog),
    network(new QNetworkAccessManager),
    calc(new Calculator),
    settings(new QSettings(C::confFileName(), QSettings::IniFormat)),
    setwindow(new Settings)

{
    ui->setupUi(this);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    initTable();

    lesson_file = QString("");
    file_is_changed = false;

    connect(schedule, &QStandardItemModel::dataChanged, this, &MainWindow::setChanged);
    connect(ui->action_settings, &QAction::triggered, setwindow, &Settings::show);
    connect(setwindow, &Settings::applied, this, &MainWindow::updateSettings);
    connect(network, &QNetworkAccessManager::finished, this, &MainWindow::getResponse);
    connect(calc, &Calculator::scheduleCalculated, this, &MainWindow::getScheduleFromCalculator);

    device_address = settings->value(C::ADDRESS).toString();
    time_sync = settings->value(C::TIME_SYNC).toBool();

    if (time_sync)
        putTimeFromPC();
}

void MainWindow::criticalError(QString err)
{
    QMessageBox::critical(this, "Ошибка", err);
}

void MainWindow::initTable()
{
    ui->tableView->setModel(nullptr);
    schedule->clear();

    if (is_calculator) {
        schedule->setColumnCount(4);
        schedule->setHorizontalHeaderLabels(
                    QStringList()
                    << "Начало"
                    << "Длина урока (мин)"
                    << "Длина перемены (мин)"
                    << "Особые звонки"
                    );
    } else {
        schedule->setColumnCount(3);
        schedule->setHorizontalHeaderLabels(
                    QStringList()
                    << "Начало"
                    << "Конец"
                    << "Особые звонки"
                    );
    }

    schedule->setRowCount(1);
    ui->tableView->setModel(schedule);
}

QByteArray MainWindow::requestPassword()
{
    return request_password(this).toUtf8();
}

QString MainWindow::askForSaveFileName()
{
    QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), "*.shdl");

    if (!filename.endsWith(".shdl"))
        filename += ".shdl";

    return filename;
}

int MainWindow::askForSaveSchedule()
{
    if (!file_is_changed)
        return QMessageBox::No;

    return QMessageBox::warning(
                this,
                "Система автоматической подачи звонков",
                "Сохранить расписание?",
                QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel
                );
}

bool MainWindow::askForQuitFromCalculatorMode()
{
    if (!is_calculator)
        return false;

    return QMessageBox::question(
                this,
                "Система автоматической подачи звонков",
                "Выйти из режима калькулятора?"
                ) == QMessageBox::Yes;
}

void MainWindow::doRequest(QString method, QString args = QString())
{
    QNetworkRequest req;

    QString url = "http://" + device_address + "/autoring";
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray data;

    data.append("method=");
    data.append(method.toUtf8());
    if (method == "set" || method == "doring") {
        data.append("&pwdhash=");
        data.append(requestPassword());
    }
    if (!args.isEmpty()) {
        data.append("&");
        data.append(args.toUtf8());
    }

    network->post(req, data);
}

void MainWindow::putTimeFromPC()
{
    QDateTime curtime = QDateTime::currentDateTime();
    QString curtime_string = QString("time=") + curtime.toString("HH:mm:ss").replace(":", "%3A");

    doRequest("set", curtime_string);
}

void MainWindow::updateSettings() {
    device_address = settings->value(C::ADDRESS).toString();
    time_sync = settings->value(C::TIME_SYNC).toBool();

    QString arg = QString("passwd=%1&pwdsalt=%2")
            .arg(setwindow->password_hash)
            .arg(QString::fromUtf8(setwindow->password_salt));

    doRequest("set", arg);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    int answer = askForSaveSchedule();

    if (answer == QMessageBox::Cancel) {
        event->ignore();
        return;
    }

    if (answer == QMessageBox::Yes)
        saveSchedule();

    calc->close();
    setwindow->close();
    event->accept();
}

void MainWindow::setChanged()
{
    file_is_changed = true;
}

void MainWindow::checkSchedule()
{
    ScheduleError e;

    if (!is_calculator)
        e = check_schedule(schedule_from_qstdim(schedule));
    else
        e = check_schedule(cmschedule_to_schedule(cmschedule_from_qstdim(schedule)));

    switch (e.error_type) {
    case SE_INCORRECT_ORDER:
        ui->statusBar->showMessage("В расписании нарушен порядок уроков");
        break;
    case SE_END_BEFORE_BEGIN:
        ui->statusBar->showMessage(
                    QString("Урок %0: конец урока раньше его начала")
                    .arg(e.error_row));
        break;
    case SE_CORRECT:
        ui->statusBar->showMessage("Расписание составлено правильно");
        break;
    case SE_NO_BEGIN_AND_END:
        ui->statusBar->showMessage(
                    QString("Урок %0: отсутствуют временные границы").arg(e.error_row));
    }
}

void MainWindow::loadSchedule()
{
    QString filename = QFileDialog::getOpenFileName(this, QString(), QString(), "*.shdl");
    if (filename.isEmpty())
        return;

    loadScheduleFromFile(filename);
}

void MainWindow::loadScheduleFromFile(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        criticalError(QString("Файл по адресу %0 не обнаружен").arg(filename));
        return;
    }

    QTextStream fin(&file);
    ui->tableView->setModel(nullptr);

    if (!is_calculator)
        schedule_to_qstdim(&schedule, csv_to_schedule(fin.readAll()));
    else
        cmschedule_to_qstdim(&schedule, csv_to_cmschedule(fin.readAll()));

    ui->tableView->setModel(schedule);
    file.close();

    lesson_file = filename;
    file_is_changed = false;
}

void MainWindow::getScheduleFromCalculator(Schedule sch)
{
    ui->tableView->setModel(nullptr);
    if (!is_calculator)
        schedule_to_qstdim(&schedule, sch);
    else
        cmschedule_to_qstdim(&schedule, schedule_to_cmschedule(sch));
    ui->tableView->setModel(schedule);
    ui->statusBar->showMessage("Расписание уроков рассчитано");
}

void MainWindow::saveScheduleToFile(QString filename)
{
    ui->tableView->setModel(nullptr);
    lesson_file = filename;

    QFile file(lesson_file);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (!lesson_file.isEmpty()) {
            criticalError(
                        QString("Файл по адресу %0 не удалось открыть или создать")
                        .arg(lesson_file));
        } else {
            criticalError("Файл не был выбран");
        }

        return;
    }

    QTextStream fout(&file);

    if (!is_calculator)
        fout << schedule_to_csv(schedule_from_qstdim(schedule));
    else
        fout << cmschedule_to_csv(cmschedule_from_qstdim(schedule));
    file.close();

    ui->tableView->setModel(schedule);
    file_is_changed = false;
}

void MainWindow::saveSchedule()
{
    ui->statusBar->clearMessage();

    if (!QFile::exists(lesson_file) || lesson_file.isEmpty()) {
        QString filename = askForSaveFileName();
        if (filename.isEmpty())
            return;

        saveScheduleToFile(filename);
    }
}

void MainWindow::saveScheduleAs()
{
    ui->statusBar->clearMessage();
    QString filename = askForSaveFileName();
    if (filename.isEmpty())
        return;

    saveScheduleToFile(filename);
}

void MainWindow::newFile()
{
    if (askForQuitFromCalculatorMode())
        turnOffCalculatorMode();

    if (askForSaveSchedule() == QMessageBox::Yes)
        saveSchedule();

    initTable();
}

void MainWindow::addLesson(int l)
{
    int rc = schedule->rowCount();
    int cc = schedule->columnCount();

    if ((rc + 1) > MAX_LESSONS) {
        ui->statusBar->showMessage(
                    QString("В расписании не может быть более %0 уроков")
                    .arg(MAX_LESSONS)
                    );
        return;
    }

    ui->statusBar->clearMessage();
    QList<QStandardItem*> newRow;

    for (int column = 0; column < cc; column++)
        newRow.append(new QStandardItem);
    if (l == -1)
        schedule->appendRow(newRow);
    else
        schedule->insertRow(l, newRow);
    newRow.clear();

    setChanged();

    if (is_calculator)
        recalculateSchedule();
}

void MainWindow::addLessonToEnd()
{
    addLesson(-1);
}

void MainWindow::addLessonBefore()
{
    int current_row = ui->tableView->selectionModel()->currentIndex().row();
    addLesson(current_row);
}

void MainWindow::addLessonAfter()
{
    int current_row = ui->tableView->selectionModel()->currentIndex().row();
    addLesson(current_row+1);
}

void MainWindow::deleteLesson(int l)
{
    int rc = schedule->rowCount();

    ui->statusBar->clearMessage();
    schedule->removeRow(l);

    if ((rc - 1) <= 0) {
        schedule->setRowCount(1);
    }

    setChanged();

    if (is_calculator)
        recalculateSchedule();
}

void MainWindow::deleteSelectedLesson()
{
    QSet<int> lessons_set;

    auto indexes = ui->tableView->selectionModel()->selectedIndexes();
    for (auto index : indexes) {
        lessons_set.insert(index.row());
    }

    QList<int> lessons = lessons_set.values();
    std::sort(lessons.begin(), lessons.end());

    for (int i = lessons.length() - 1; i >= 0; i--) {
        deleteLesson(lessons[i]);
    }
}

void MainWindow::aboutApplication()
{
    about->show();
}

void MainWindow::uploadSchedule()
{
    QString x;
    Schedule sch = (!is_calculator)
            ? schedule_from_qstdim(schedule)
            : cmschedule_to_schedule(cmschedule_from_qstdim(schedule));

    x += QString("lessnum=%0&schedule=").arg(sch.length());

    for (int i = 0; i < sch.length(); i++) {
        x += QString("%0-%1.%2.%3_")
                .arg(i+1)
                .arg(sch[i].ls_hour*60 + sch[i].ls_minute)
                .arg(sch[i].le_hour*60 + sch[i].le_minute)
                .arg(sch[i].rings);
    }

    doRequest("set", x);
}

void MainWindow::downloadSchedule()
{
    doRequest("schedule");
    setChanged();
}

void MainWindow::setTime()
{
    QTime time = get_time(this);
    if (!time.isValid())
        return;

    QString curtime_string = "time=";
    curtime_string += time.toString("HH:mm:ss").replace(":", "%3A");
 
    doRequest("set", curtime_string);
}

void MainWindow::getResponse(QNetworkReply* rp)
{
    QByteArray answer;

    if (rp->error() == QNetworkReply::NoError) {
        answer = rp->readAll();

        for (QByteArray x : QByteArray(answer).split('&')) {
            QStringList sx;
            for (QByteArray y : x.split('='))
                sx.append(QByteArray::fromPercentEncoding(y));
            if (sx[0] == "schedule") {
                Schedule sch;
                QString record;
                for (QChar x : sx[1]) {
                    if (x != '_')
                        record += x;
                    else {
                        int ln, ls, le, rn;
                        sscanf(record.toUtf8().data(), "%d-%d.%d.%d",
                               &ln, &ls, &le, &rn);
                        ScheduleEntry se;
                        se.ls_hour = ls / 60;
                        se.ls_minute = ls % 60;
                        se.le_hour = le / 60;
                        se.le_minute = le % 60;
                        se.rings = rn;

                        sch.append(se);
                        record.clear();
                    }
                }

                ui->tableView->setModel(nullptr);
                if (!is_calculator)
                    schedule_to_qstdim(&schedule, sch);
                else
                    cmschedule_to_qstdim(&schedule, schedule_to_cmschedule(sch));

                ui->tableView->setModel(schedule);
                ui->statusBar->showMessage("Расписание получено успешно");
            } else if (sx[0] == "pwdhash") {
                QString new_hash = sx[1];
                if (new_hash == "NULL")
                    new_hash = "";

                settings->setValue(C::PASSWORD_HASH, new_hash);
            } else if (sx[0] == "pwdsalt") {
                QString new_salt = sx[1];
                if (new_salt == "NULL")
                    new_salt = "";

                settings->setValue(C::PASSWORD_SALT, new_salt);
            } else if (sx[0] == "state") {
                if (sx[1] == "0")
                    ui->statusBar->showMessage("Операция выполнена успешно");
                else if (sx[1] == "1")
                    criticalError("Неверный пароль");
            }
        }
    } else {
        if (!device_address.isEmpty())
            criticalError(
                        QString("Произошла ошибка при обращении к устройству по адресу %0.")
                        .arg(device_address));
        else
            criticalError("Адрес устройства не был указан.");
    }
}

void MainWindow::invokeCalculator()
{
    calc->show();
}

void MainWindow::doRings()
{
    RingParameters r = get_ring_parameters(this);

    QString args = QString("number=%1&time=%2")
            .arg(r.ring_n)
            .arg(r.ring_d);
    if (r.pause_d >= 1) {
        args += QString("&pause=%1").arg(r.pause_d);
    }

    doRequest("doring", args);
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
    connect(action_deleteLesson, SIGNAL(triggered()), this, SLOT(deleteSelectedLesson()));

    menu->addAction(action_addBefore);
    menu->addAction(action_addAfter);
    menu->addAction(action_deleteLesson);

    menu->popup(ui->tableView->viewport()->mapToGlobal(position));
}

void MainWindow::turnOnCalculatorMode()
{
    if (is_calculator)
        return;

    ui->action_togglecm->setIcon(QIcon(":/images/simple_editing.png"));
    ui->action_togglecm->setText("Включить режим простого редактирования");

    ui->tableView->setFocus();
    ui->tableView->setModel(nullptr);

    Schedule sch = schedule_from_qstdim(schedule);
    CalcModeSchedule cmsch = schedule_to_cmschedule(sch);
    cmschedule_to_qstdim(&schedule, cmsch);

    disconnect(ui->action_togglecm, &QAction::triggered, this, &MainWindow::turnOnCalculatorMode);
    connect(ui->action_togglecm, &QAction::triggered, this, &MainWindow::turnOffCalculatorMode);
    connect(schedule, &QStandardItemModel::itemChanged, this, &MainWindow::changeBreaksAndRecalculate);

    ui->tableView->setModel(schedule);

    is_calculator = true;
}

void MainWindow::turnOffCalculatorMode()
{
    if (!is_calculator)
        return;

    ui->action_togglecm->setIcon(QIcon(":/images/calculator_mode.png"));
    ui->action_togglecm->setText("Включить режим калькулятора");

    ui->tableView->setFocus();
    ui->tableView->setModel(nullptr);

    CalcModeSchedule cmsch = cmschedule_from_qstdim(schedule);
    Schedule sch = cmschedule_to_schedule(cmsch);
    schedule_to_qstdim(&schedule, sch);

    disconnect(ui->action_togglecm, &QAction::triggered, this, &MainWindow::turnOffCalculatorMode);
    connect(ui->action_togglecm, &QAction::triggered, this, &MainWindow::turnOnCalculatorMode);
    disconnect(schedule, &QStandardItemModel::itemChanged, this, &MainWindow::changeBreaksAndRecalculate);

    ui->tableView->setModel(schedule);

    is_calculator = false;
}

void MainWindow::recalculateSchedule()
{
    ui->tableView->setFocus();
    ui->tableView->setModel(nullptr);

    int start;
    int first_lesson_start = qstring_to_minutes(schedule->index(0, 0).data().toString());

    start = first_lesson_start;

    QList<QStandardItem*> start_column;
    start_column.append(
                new QStandardItem(minutes_to_qstring(first_lesson_start))
                );

    QTime start_element(0, 0);
    int rc = schedule->rowCount();
    for (int row = 1; row < rc; row++) {
        start += schedule->index(row-1, 1).data().toInt();
        start += schedule->index(row-1, 2).data().toInt();
        start_column.append(new QStandardItem(minutes_to_qstring(start)));
    }

    schedule->removeColumn(0);
    schedule->insertColumn(0, start_column);
    schedule->setHorizontalHeaderItem(0, new QStandardItem("Начало"));

    ui->tableView->setModel(schedule);
}

void MainWindow::changeBreaksAndRecalculate(QStandardItem* item)
{
    int rn = item->row();
    int cn = item->column();

    if ((cn == 0) && (rn > 0)) {
        ui->tableView->setFocus();
        ui->tableView->setModel(nullptr);

        int lesson_delay = schedule->index(rn, 1).data().toInt();
        int break_delay = schedule->index(rn-1, 2).data().toInt();
        int start_time =
                qstring_to_minutes(schedule->index(rn, 0).data().toString());
        int start1_time =
                qstring_to_minutes(schedule->index(rn-1, 0).data().toString());

        break_delay += start_time - (start1_time+break_delay) - lesson_delay;

        if (break_delay <= 0) {
            ui->tableView->setModel(schedule);
            criticalError("Перемена полностью отсутствует или пересекается с уроком");
            this->recalculateSchedule();
            return;
        }

        QList<QStandardItem*> breaks;
        breaks = schedule->takeColumn(2);
        breaks[rn-1] = new QStandardItem(QString::number(break_delay));

        schedule->removeColumn(3);
        schedule->insertColumn(2, breaks);
        schedule->setHorizontalHeaderItem(2, new QStandardItem("Длина перемены (мин)"));

        ui->tableView->setModel(schedule);
    }

    this->recalculateSchedule();
}

void MainWindow::getPassword()
{
    doRequest("password");
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
