#include "calculator.h"
#include "ui_calculator.h"

#include <QDebug>

Calculator::Calculator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Calculator),
    breaks(new QStandardItemModel)
{
    ui->setupUi(this);
    ui->break_delays->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    breaks->setColumnCount(1);
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены после урока (мин)");

    breaks->setRowCount(ui->lesson_quantity->value()-1);

    ui->break_delays->setModel(breaks);
}

void Calculator::calculateSchedule()
{
    QTime first_lesson = ui->first_lesson_start->time();
    int lesson_delay = ui->lesson_delay->value();
    int lesson_quantity = ui->lesson_quantity->value();

    QPair<QTime, QTime> time_range;
    time_range.first = first_lesson;
    time_range.second = time_range.first.addSecs(lesson_delay*60);

    QList<int> brk;
    for (int i = 0; i < breaks->rowCount(); i++) {
        QModelIndex ind = breaks->index(i, 0);
        brk.append(ind.data().toInt());
    }

    schedule.resize(lesson_quantity);
    schedule.fill(time_range);

    for (int i = 1; i < lesson_quantity; i++) {
        time_range.first = schedule[i-1].second.addSecs(brk[i-1]*60);
        time_range.second = time_range.first.addSecs(lesson_delay*60);
        schedule[i] = time_range;
    }
}

void Calculator::saveSchedule()
{
    calculateSchedule();

    QStringList rows;
    for (int i = 0; i < schedule.size(); i++) {
        rows.push_back("");
        rows[i].sprintf("%02d,%02d:%02d,%02d:%02d,0", i+1,
                    schedule[i].first.hour(), schedule[i].first.minute(),
                    schedule[i].second.hour(), schedule[i].second.minute());

    }

    emit scheduleMade(rows);
    close();
}

void Calculator::resizeSchedule(int lc)
{
    breaks->setRowCount(lc-1);
}

void Calculator::equalize()
{
    ui->break_delays->setModel(nullptr);

    breaks->clear();
    breaks->setColumnCount(1);
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены после урока (мин)");

    int break_delay = ui->break_delay->value();
    QList<QStandardItem*> row;
    for (int i = 0; i < ui->lesson_quantity->value()-1; i++) {
        row.append(new QStandardItem(QString::number(break_delay)));
        breaks->appendRow(row);
        row.clear();
    }

    ui->break_delays->setModel(breaks);
}

void Calculator::shorten()
{
    ui->break_delays->setModel(nullptr);

    int delta = ui->break_delta->value();
    QModelIndex ind;
    QList<int> brk;
    for (int i = 0; i < breaks->rowCount(); i++) {
        ind = breaks->index(i, 0);
        brk.append(ind.data().toInt()-delta);
    }

    breaks->clear();
    breaks->setColumnCount(1);
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены после урока (мин)");

    QList<QStandardItem*> row;
    for (int i = 0; i < brk.size(); i++) {
        row.append(new QStandardItem(QString::number(brk[i])));
        breaks->appendRow(row);
        row.clear();
    }

    ui->break_delays->setModel(breaks);
}

void Calculator::lengthen()
{
    ui->break_delays->setModel(nullptr);

    int delta = ui->break_delta->value();
    QModelIndex ind;
    QList<int> brk;
    for (int i = 0; i < breaks->rowCount(); i++) {
        ind = breaks->index(i, 0);
        brk.append(ind.data().toInt()+delta);
    }

    breaks->clear();
    breaks->setColumnCount(1);
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены после урока (мин)");

    QList<QStandardItem*> row;
    for (int i = 0; i < brk.size(); i++) {
        row.append(new QStandardItem(QString::number(brk[i])));
        breaks->appendRow(row);
        row.clear();
    }
    ui->break_delays->setModel(breaks);
}

Calculator::~Calculator()
{
    delete breaks;
    delete ui;
}
