#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load("qt_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&translator);

    MainWindow w;

    QSettings settings("configuration.ini", QSettings::IniFormat);
    QString correct_passwd = QString(QByteArray::fromBase64(settings.value("password").toByteArray()));

    QFile session_lock("session.lock");

    QString passwd;
    bool typed;
    if ((correct_passwd != "") && !(session_lock.exists())) {
        while (true) {
            passwd = QInputDialog::getText(
                        nullptr, "Пароль", "Введите пароль системы: ", QLineEdit::Password,
                        QString(), &typed);
            if (!typed) return 0;
            if (passwd != correct_passwd) {
                QMessageBox::critical(nullptr, "Ошибка", "Неверный пароль");
            } else {
                if (session_lock.open(QIODevice::WriteOnly | QIODevice::Append)) {
                    session_lock.write("SL\n");
                    session_lock.close();
                }
                break;
            }
        }
    }

    w.show();

    if ((argc > 1) && (QFile::exists(argv[1]))) {
        w.loadScheduleFromFile(argv[1]);
    } else if ((argc > 1) && !(QFile::exists(argv[1]))) {
        w.hide();
        QMessageBox::critical(nullptr, "Ошибка", "Файла не существует");
    }

    int ret = a.exec();
    if (ret == 0) {
        session_lock.remove();
    }
    return a.exec();
}
