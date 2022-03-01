#include "mainwindow.h"

#include <QtGlobal>
#include <QApplication>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

#include "passwdtools.h"

#if (QT_VERSION >= 0x060000)
#define QLIBRARYINFO_PATH QLibraryInfo::path
#else
#define QLIBRARYINFO_PATH QLibraryInfo::location
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString fileName = QString::fromLocal8Bit(argv[1]);

    QTranslator translator;
    if (translator.load("qt_ru", QLIBRARYINFO_PATH(QLibraryInfo::TranslationsPath)))
        a.installTranslator(&translator);
    else {
        QMessageBox::critical(nullptr, "Error", "Translations have not been loaded");
        return -1;
    }

    MainWindow w;

    QSettings settings(QDir::homePath() + "/.autoringrc.ini", QSettings::IniFormat);
    QFile session_lock(QDir::homePath() + "/.session");

    PasswordCheckState typed;

    while ((typed = check_password())) {
        if (typed == PASS_CORRECT) {
            if (session_lock.open(QIODevice::WriteOnly | QIODevice::Append)) {
                session_lock.write("SL\n");
                session_lock.close();
            }
            break;
        } else if (typed == PASS_INCORRECT) {
            QMessageBox::critical(nullptr, "Ошибка", "Неверный пароль");
        } else {
            return 0;
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
