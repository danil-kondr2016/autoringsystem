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

    QString passwd;
    bool typed;
    if (correct_passwd != "") {
        while (true) {
            passwd = QInputDialog::getText(
                        nullptr, "Пароль", "Введите пароль системы: ", QLineEdit::Password,
                        QString(), &typed);
            if (!typed) return 0;
            if (passwd != correct_passwd) {
                QMessageBox::critical(nullptr, "Ошибка", "Неверный пароль");
            } else {
                break;
            }
        }
    }

    w.show();

    return a.exec();
}
