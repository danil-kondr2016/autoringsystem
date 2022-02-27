#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTime>

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "scheduletools.h"
#include "constants.h"

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

Q_SIGNALS:
    void scheduleCalculated(Schedule sch);

private:
    Ui::Calculator *ui;
};

#endif // CALCULATOR_H
