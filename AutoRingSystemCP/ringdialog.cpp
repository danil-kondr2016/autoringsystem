#include "ringdialog.h"
#include "ui_ringdialog.h"

RingDialog::RingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RingDialog)
{
    ui->setupUi(this);
    setModal(true);
}

void RingDialog::initialize(int q, int t, int p)
{
    ui->ringQuantity->setValue(q);
    ui->ringDelay->setValue(t);
    ui->pauseTime->setValue((q >= 2) ? p : 0);

    if (q < 2) {
        ui->pauseTime->setEnabled(false);
        ui->label_3->setEnabled(false);
    } else {
        ui->pauseTime->setEnabled(true);
        ui->label_3->setEnabled(true);
    }

    quantity = q;
    time = t;
    pause = (q >= 2) ? p : 0;
}

void RingDialog::quantityChanged(int value)
{
    quantity = value;
    if (value < 2) {
        ui->pauseTime->setEnabled(false);
        ui->label_3->setEnabled(false);
        pause = 0;
    } else {
        ui->pauseTime->setEnabled(true);
        ui->label_3->setEnabled(true);
    }
}

void RingDialog::timeChanged(int value)
{
    time = value;
}

void RingDialog::pauseChanged(int value)
{
    pause = value;
}

RingDialog::~RingDialog()
{
    delete ui;
}
