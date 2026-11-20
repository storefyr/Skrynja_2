#include <QCoreApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

#include "logfil.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("harbour");
    QCoreApplication::setApplicationName("harbour-skrynja-2");

    Logfil::instans()->skrivLinje("start");

    QString qmlSti = QCoreApplication::applicationDirPath()
                     + "/../share/harbour-skrynja-2/qml/harbour-skrynja-2.qml";
    if (!QFileInfo::exists(qmlSti))
        qmlSti = QCoreApplication::applicationDirPath() + "/qml/harbour-skrynja-2.qml";
    if (!QFileInfo::exists(qmlSti))
        qmlSti = "qml/harbour-skrynja-2.qml";

    if (!QFileInfo::exists(qmlSti))
        return 0;

    QQmlApplicationEngine engine;
    engine.load(QUrl::fromLocalFile(qmlSti));
    if (engine.rootObjects().isEmpty())
        return 1;
    return app.exec();
}
