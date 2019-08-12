/*

Calculator.cpp

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

#include "calculator.h"
#include "ui_calculator.h"

Calculator::Calculator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Calculator),
    breaks(new QStandardItemModel)
{
    ui->setupUi(this);
    breaks->setColumnCount(1);
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены в минутах");

    int lq = ui->lesson_quantity->value();
    QList<QStandardItem*> row;
    for (int i = 0; i < lq; i++) {
        row.append(new QStandardItem(QString::number(lq)));
        breaks->appendRow(row);
        row.clear();
    }


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
        rows[i].sprintf("%d,%02d:%02d,%02d:%02d,0", i+1,
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
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены в минутах");

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
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены в минутах");

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
    breaks->setHorizontalHeaderLabels(QStringList() << "Длительность перемены в минутах");

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
