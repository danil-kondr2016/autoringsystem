#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

#include "passwdtools.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString fileName = QString::fromLocal8Bit(argv[1]);

    QTranslator translator;
    translator.load("qt_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&translator);

    MainWindow w;

    QSettings settings(QDir::homePath() + "/.autoringrc.ini", QSettings::IniFormat);
    QByteArray password_hash = settings.value("password_hash").toByteArray();
    QByteArray password_salt = settings.value("password_salt").toByteArray();

    QFile session_lock(QDir::homePath() + "/.session");

    QString passwd;
    bool typed;
    if ((password_salt != QByteArray() && password_hash != QByteArray()) && !(session_lock.exists())) {
        while (true) {
            passwd = QInputDialog::getText(
                        nullptr, "Пароль", "Введите пароль системы: ", QLineEdit::Password,
                        QString(), &typed);
            if (!typed) return 0;

            QString hash = get_password_hash(passwd.toUtf8(), password_salt);

            if (hash != password_hash) {
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

    if ((argc > 1) && (QFile::exists(fileName))) {
        w.loadScheduleFromFile(fileName);
    } else if ((argc > 1) && !(QFile::exists(fileName))) {
        w.hide();
        QMessageBox::critical(nullptr, "Ошибка", QString("Файла %1 не существует").arg(fileName));
        w.show();
    }

    int ret = a.exec();
    if (ret == 0) {
        session_lock.remove();
    }
    return ret;
}
