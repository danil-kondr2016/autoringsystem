#include "ringdialog.h"
#include "ui_ringdialog.h"

RingDialog::RingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RingDialog)
{
    ui->setupUi(this);
    setModal(true);
}

void RingDialog::initialize(int q, int d, int p)
{
    ui->ringQuantity->setValue(q);
    ui->ringDuration->setValue(d);
    ui->pauseDuration->setValue((q >= 2) ? p : 0);

    if (q < 2) {
        ui->pauseDuration->setEnabled(false);
        ui->label_3->setEnabled(false);
    } else {
        ui->pauseDuration->setEnabled(true);
        ui->label_3->setEnabled(true);
    }

    quantity = q;
    duration = d;
    pause = (q >= 2) ? p : 0;
}

void RingDialog::quantityChanged(int value)
{
    quantity = value;
    if (value < 2) {
        ui->pauseDuration->setEnabled(false);
        ui->label_3->setEnabled(false);
        pause = 0;
    } else {
        ui->pauseDuration->setEnabled(true);
        ui->label_3->setEnabled(true);
    }
}

void RingDialog::durationChanged(int value)
{
    duration = value;
}

void RingDialog::pauseChanged(int value)
{
    pause = value;
}

RingDialog::~RingDialog()
{
    delete ui;
}

RingParameters get_ring_parameters(QWidget * parent = nullptr)
{
    RingParameters result = {0,0,0,0};

    RingDialog* ringDialog = new RingDialog(parent);
    ringDialog->initialize(1, 3);

    result.result = ringDialog->exec();

    if (result.result == QDialog::Accepted) {
        result.ring_n = ringDialog->quantity;
        result.ring_d = ringDialog->duration;
        result.pause_d = ringDialog->pause;
    }

    delete ringDialog;
    return result;
}
