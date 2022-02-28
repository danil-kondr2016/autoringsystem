#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "constants.h"

#include <QMainWindow>

#include <QDateTime>
#include <QDate>
#include <QTime>

#include <QtEvents>
#include <QEvent>
#include <QCloseEvent>

#include <QTextStream>
#include <QStandardItemModel>

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include <QTime>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include <QSettings>

#include "aboutdialog.h"
#include "settings.h"
#include "calculator.h"
#include "ringdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString lesson_file;
    bool file_is_changed;

    QString device_ip_address;
    bool time_sync;

    bool is_calculator = false;

    void initTable();

    void doRequest(QString method, QString args);
    QString askForSaveFileName();
    int askForSaveSchedule();
    bool askForQuitFromCalculatorMode();
    void closeEvent(QCloseEvent* event);

    void loadScheduleFromFile(QString filename);
    void saveScheduleToFile(QString filename);
    QByteArray requestPassword();

    void addLesson(int l);
    void deleteLesson(int l);

public Q_SLOTS:
    void newFile();

    void saveSchedule();
    void saveScheduleAs();
    void loadSchedule();
    void checkSchedule();

    void uploadSchedule();
    void downloadSchedule();
    void getScheduleFromCalculator(Schedule sch);

    void addLessonToEnd();
    void addLessonBefore();
    void addLessonAfter();

    void deleteSelectedLesson();

    void putTimeFromPC();
    void setTime();
    void updateSettings();

    void aboutApplication();

    void setChanged();
    void getResponse(QNetworkReply* rp);

    void invokeCalculator();

    void doRings();

    void tableViewContextMenu(QPoint position);

    void turnOnCalculatorMode();
    void turnOffCalculatorMode();

    void changeBreaksAndRecalculate(QStandardItem* item);
    void recalculateSchedule();

    void getPassword();

private:    
    Ui::MainWindow *ui;
    QStandardItemModel *schedule;
    AboutDialog *about;
    QNetworkAccessManager *network;
    Calculator *calc;

    QSettings *settings;
    Settings *setwindow;
};

#endif // MAINWINDOW_H
