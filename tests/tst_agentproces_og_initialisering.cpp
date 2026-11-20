#include <QtTest/QtTest>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "acpclient.h"
#include "agentproces.h"
#include "testregistrering.h"

class TestAgentprocesOgInitialisering : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void standard_agentsti_er_vibe_acp();
    void initialize_payload_protocolversion_1();
    void initialize_payload_tom_clientCapabilities();
    void initialize_svar_parses_og_giver_config_options();
    void startfejl_giver_lastError_uden_haeng();
};

REGISTRER_TEST(TestAgentprocesOgInitialisering)

void TestAgentprocesOgInitialisering::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-agent";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

void TestAgentprocesOgInitialisering::standard_agentsti_er_vibe_acp()
{
    QSettings s;
    s.remove("agent/sti");
    s.sync();
    AgentProces proces;
    QCOMPARE(proces.agentsti(), QString("/home/defaultuser/.local/bin/vibe-acp"));
}

void TestAgentprocesOgInitialisering::initialize_payload_protocolversion_1()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    QSignalSpy sendt(&klient, SIGNAL(beskedSendt(QByteArray)));
    klient.sendInitialize();
    QCOMPARE(sendt.count(), 1);
    QJsonObject obj = QJsonDocument::fromJson(sendt.at(0).at(0).toByteArray()).object();
    QCOMPARE(obj.value("method").toString(), QString("initialize"));
    QCOMPARE(obj.value("params").toObject().value("protocolVersion").toInt(), 1);
}

void TestAgentprocesOgInitialisering::initialize_payload_tom_clientCapabilities()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    QSignalSpy sendt(&klient, SIGNAL(beskedSendt(QByteArray)));
    klient.sendInitialize();
    QJsonObject params = QJsonDocument::fromJson(sendt.at(0).at(0).toByteArray()).object()
                             .value("params").toObject();
    QJsonValue cap = params.value("clientCapabilities");
    QVERIFY(cap.isObject());
    QVERIFY(cap.toObject().isEmpty());
}

void TestAgentprocesOgInitialisering::initialize_svar_parses_og_giver_config_options()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    QSignalSpy sendt(&klient, SIGNAL(beskedSendt(QByteArray)));
    QSignalSpy klar(&klient, SIGNAL(agentKlar()));
    klient.sendInitialize();
    qint64 id = QJsonDocument::fromJson(sendt.at(0).at(0).toByteArray()).object().value("id").toVariant().toLongLong();
    QJsonObject resultat;
    resultat.insert("protocolVersion", 1);
    QJsonArray options;
    QJsonObject opt;
    opt.insert("configId", "model");
    opt.insert("name", "Model");
    opt.insert("value", "vibe-large");
    options.append(opt);
    resultat.insert("configOptions", options);
    QJsonObject svar;
    svar.insert("jsonrpc", "2.0");
    svar.insert("id", QJsonValue(id));
    svar.insert("result", resultat);
    klient.feedBytesForTest(QJsonDocument(svar).toJson(QJsonDocument::Compact) + "\n");
    QCOMPARE(klar.count(), 1);
    QCOMPARE(klient.configOptions().size(), 1);
    QCOMPARE(klient.configOptions().at(0).toMap().value("configId").toString(), QString("model"));
}

void TestAgentprocesOgInitialisering::startfejl_giver_lastError_uden_haeng()
{
    AgentProces proces;
    proces.sætAgentsti("/definitivt/ikke/eksisterende-agent");
    QElapsedTimer ur;
    ur.start();
    QVERIFY(!proces.start());
    QVERIFY(ur.elapsed() < 2000);
    QVERIFY(!proces.lastError().isEmpty());
    QVERIFY(!proces.kører());

    AcpClient klient;
    klient.agentProces()->sætAgentsti("/definitivt/ikke/eksisterende-agent");
    QVERIFY(!klient.startAgent());
    QVERIFY(!klient.lastError().isEmpty());
}

#include "tst_agentproces_og_initialisering.moc"
