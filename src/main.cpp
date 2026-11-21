#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

#include "acpclient.h"
#include "logfil.h"
#include "selvtest.h"

int main(int argc, char *argv[])
{
    bool selfTest = false;
    for (int i = 1; i < argc; ++i) {
        if (QByteArray(argv[i]) == "--selvtest")
            selfTest = true;
    }
    if (selfTest) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setOrganizationName("harbour");
        QCoreApplication::setApplicationName("harbour-skrynja-2");
        LogFile::instance()->writeLine("selfTest");
        AcpClient client;
        QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (base.isEmpty())
            base = QDir::tempPath();
        QString verdictPath = base + "/selvtest-verdict.json";
        SelfTest selfTest(&client);
        QObject::connect(&selfTest, &SelfTest::finished, &app, [](int kode) {
            QCoreApplication::exit(kode);
        });
        selfTest.start("selvtest: skriv en fil og afslut runden", verdictPath);
        return app.exec();
    }

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("harbour");
    QCoreApplication::setApplicationName("harbour-skrynja-2");

    LogFile::instance()->writeLine("start");

    AcpClient client;

    QString qmlPath = QCoreApplication::applicationDirPath()
                     + "/../share/harbour-skrynja-2/qml/harbour-skrynja-2.qml";
    if (!QFileInfo::exists(qmlPath))
        qmlPath = QCoreApplication::applicationDirPath() + "/qml/harbour-skrynja-2.qml";
    if (!QFileInfo::exists(qmlPath))
        qmlPath = "qml/harbour-skrynja-2.qml";

    if (!QFileInfo::exists(qmlPath))
        return 0;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("acp", &client);
    engine.load(QUrl::fromLocalFile(qmlPath));
    if (engine.rootObjects().isEmpty())
        return 1;
    client.startAgent();
    return app.exec();
}
