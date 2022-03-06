#ifndef TIMEDIALOG_H
#define TIMEDIALOG_H

#include <QTime>
#include <QDialog>

QTime get_time(QWidget * parent);

namespace Ui {
class TimeDialog;
}

class TimeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeDialog(QWidget *parent = nullptr);
    ~TimeDialog();

    QTime time;

private:
    Ui::TimeDialog *ui;

private Q_SLOTS:
    void setTime(QTime time);
};

#endif // TIMEDIALOG_H
