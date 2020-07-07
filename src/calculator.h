/*

Calculator.h

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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTime>

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

namespace Ui {
class Calculator;
}

class Calculator : public QMainWindow
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator();

    QVector<QPair<QTime, QTime>> schedule;

public Q_SLOTS:
    void calculateSchedule();
    void saveSchedule();

    void resizeSchedule(int lc);
    void equalize();
    void shorten();
    void lengthen();

Q_SIGNALS:
    void scheduleMade(QStringList rows);

private:
    Ui::Calculator *ui;
    QStandardItemModel *breaks;
};

#endif // CALCULATOR_H
