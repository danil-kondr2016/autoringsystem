#ifndef RINGDIALOG_H
#define RINGDIALOG_H

#include <QDialog>

struct RingParameters {
    int ring_n, ring_d, pause_d;
    int result;
};

RingParameters get_ring_parameters(QWidget * parent);

namespace Ui {
class RingDialog;
}

class RingDialog : public QDialog
{
    Q_OBJECT

public:
    int quantity, duration, pause;

    explicit RingDialog(QWidget *parent = nullptr);
    ~RingDialog();

public Q_SLOTS:
    void initialize(int q, int t, int p = 0);

private Q_SLOTS:
    void quantityChanged(int value);
    void durationChanged(int value);
    void pauseChanged(int value);

private:
    Ui::RingDialog *ui;
};

#endif // RINGDIALOG_H
