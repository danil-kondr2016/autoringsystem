#ifndef RINGDIALOG_H
#define RINGDIALOG_H

#include <QDialog>

namespace Ui {
class RingDialog;
}

class RingDialog : public QDialog
{
    Q_OBJECT

public:
    int quantity, time, pause;

    explicit RingDialog(QWidget *parent = nullptr);
    ~RingDialog();

public Q_SLOTS:
    void initialize(int q, int t, int p = 0);

private Q_SLOTS:
    void quantityChanged(int value);
    void timeChanged(int value);
    void pauseChanged(int value);

private:
    Ui::RingDialog *ui;
};

#endif // RINGDIALOG_H
