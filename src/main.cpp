#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qt3d-demo_" + QLocale(locale).name();
        bool loaded = translator.load(":/i18n/" + baseName);
        if (!loaded) {
            loaded = translator.load(baseName, QCoreApplication::applicationDirPath() + "/i18n");
        }
        if (loaded) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
