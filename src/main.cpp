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
    bool selvtest = false;
    for (int i = 1; i < argc; ++i) {
        if (QByteArray(argv[i]) == "--selvtest")
            selvtest = true;
    }
    if (selvtest) {
        QCoreApplication app(argc, argv);
        QCoreApplication::setOrganizationName("harbour");
        QCoreApplication::setApplicationName("harbour-skrynja-2");
        Logfil::instans()->skrivLinje("selvtest");
        AcpClient klient;
        QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (base.isEmpty())
            base = QDir::tempPath();
        QString verdictSti = base + "/selvtest-verdict.json";
        Selvtest selvtest(&klient);
        QObject::connect(&selvtest, &Selvtest::faerdig, &app, [](int kode) {
            QCoreApplication::exit(kode);
        });
        selvtest.start("selvtest: skriv en fil og afslut runden", verdictSti);
        return app.exec();
    }

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("harbour");
    QCoreApplication::setApplicationName("harbour-skrynja-2");

    Logfil::instans()->skrivLinje("start");

    AcpClient klient;

    QString qmlSti = QCoreApplication::applicationDirPath()
                     + "/../share/harbour-skrynja-2/qml/harbour-skrynja-2.qml";
    if (!QFileInfo::exists(qmlSti))
        qmlSti = QCoreApplication::applicationDirPath() + "/qml/harbour-skrynja-2.qml";
    if (!QFileInfo::exists(qmlSti))
        qmlSti = "qml/harbour-skrynja-2.qml";

    if (!QFileInfo::exists(qmlSti))
        return 0;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("acp", &klient);
    engine.load(QUrl::fromLocalFile(qmlSti));
    if (engine.rootObjects().isEmpty())
        return 1;
    klient.startAgent();
    return app.exec();
}
