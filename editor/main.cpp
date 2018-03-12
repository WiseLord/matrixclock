#include "mainwindow.h"
#include "defines.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    QApplication a(argc, argv);

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    QString lang = settings.value(SETTINGS_GENERAL_LANGUAGE, "auto").toString();

    if (lang.compare("auto") == 0) {
        lang = QLocale::system().bcp47Name().remove(QRegExp("-.*"));
    }
/*
    QTranslator translator;
    translator.load(":/ts/editor_" + lang);
    a.installTranslator(&translator);
*/
    MainWindow w;
    w.show();

    return a.exec();
}
