#include "calculator.h"
#include "ui_calculator.h"

#include <QDebug>

Calculator::Calculator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Calculator)
{
    ui->setupUi(this);
    ui->lesson_quantity->setMaximum(MAX_LESSONS);
}

void Calculator::calculateSchedule()
{
    QTime first_lesson = ui->first_lesson_start->time();
    int lesson_length = ui->lesson_length->value();
    int lesson_quantity = ui->lesson_quantity->value();
    int short_break_length = ui->short_break_length->value();
    int long_break_length = ui->long_break_length->value();

    QPair<QTime, QTime> time_range;
    time_range.first = first_lesson;
    time_range.second = time_range.first.addSecs(lesson_length*60);

    QList<int> brk;
    QStringList brk_strs = ui->long_breaks->text().split(",");

    for (int i = 0; i < lesson_quantity-1; i++) {
        brk.append(short_break_length);
    }

    for (QString x : brk_strs) {
        int a = x.toInt() - 1;
        brk[a] = long_break_length;
    }

    schedule.resize(lesson_quantity);
    schedule.fill(time_range);

    for (int i = 1; i < lesson_quantity; i++) {
        time_range.first = schedule[i-1].second.addSecs(brk[i-1]*60);
        time_range.second = time_range.first.addSecs(lesson_length*60);
        schedule[i] = time_range;
    }
}

void Calculator::saveSchedule()
{
    calculateSchedule();

    Schedule sch;
    ScheduleEntry se;

    se.is_incorrect = 0;
    se.rings = 0;

    for (int i = 0; i < schedule.size(); i++) {
        se.ls_hour = schedule[i].first.hour();
        se.ls_minute = schedule[i].first.minute();
        se.le_hour = schedule[i].second.hour();
        se.le_minute = schedule[i].second.minute();
        sch.push_back(se);
    }

    emit scheduleCalculated(sch);
    close();
}

Calculator::~Calculator()
{
    delete ui;
}
