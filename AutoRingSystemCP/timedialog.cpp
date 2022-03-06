#include "timedialog.h"
#include "ui_timedialog.h"

#include <QDateTime>

TimeDialog::TimeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeDialog)
{
    ui->setupUi(this);
    setModal(true);

    ui->timeSelect->setTime(QTime::currentTime());
}

void TimeDialog::setTime(QTime t) {
    this->time = t;
}

TimeDialog::~TimeDialog()
{
    delete ui;
}

QTime get_time(QWidget * parent = nullptr)
{
    TimeDialog * dlg = new TimeDialog(parent);

    int result = dlg->exec();
    if (result == QDialog::Accepted) {
        return dlg->time;
    } else {
        return QTime();
    }
}
