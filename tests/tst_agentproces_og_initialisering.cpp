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

REGISTER_TEST(TestAgentprocesOgInitialisering)

void TestAgentprocesOgInitialisering::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-agent");
}

void TestAgentprocesOgInitialisering::standard_agentsti_er_vibe_acp()
{
    QCOMPARE(testStandardAgentsti(), QString("/home/defaultuser/.local/bin/vibe-acp"));
}

void TestAgentprocesOgInitialisering::initialize_payload_protocolversion_1()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy sendt(&client, SIGNAL(messageSent(QByteArray)));
    client.sendInitialize();
    QCOMPARE(sendt.count(), 1);
    QJsonObject obj = QJsonDocument::fromJson(sendt.at(0).at(0).toByteArray()).object();
    QCOMPARE(obj.value("method").toString(), QString("initialize"));
    QCOMPARE(obj.value("params").toObject().value("protocolVersion").toInt(), 1);
}

void TestAgentprocesOgInitialisering::initialize_payload_tom_clientCapabilities()
{
    AcpClient client;
    injectTestModels(&client, this);
    QJsonValue cap = testInitializeParams(client).value("clientCapabilities");
    QVERIFY(cap.isObject());
    QVERIFY(cap.toObject().isEmpty());
}

void TestAgentprocesOgInitialisering::initialize_svar_parses_og_giver_config_options()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy sendt(&client, SIGNAL(messageSent(QByteArray)));
    QSignalSpy klar(&client, SIGNAL(agentReady()));
    client.sendInitialize();
    qint64 id = QJsonDocument::fromJson(sendt.at(0).at(0).toByteArray()).object().value("id").toVariant().toLongLong();
    QJsonObject result;
    result.insert("protocolVersion", 1);
    QJsonArray options;
    QJsonObject opt;
    opt.insert("configId", "model");
    opt.insert("name", "Model");
    opt.insert("value", "vibe-large");
    options.append(opt);
    result.insert("configOptions", options);
    QJsonObject response;
    response.insert("jsonrpc", "2.0");
    response.insert("id", QJsonValue(id));
    response.insert("result", result);
    client.feedBytesForTest(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
    QCOMPARE(klar.count(), 1);
    QCOMPARE(client.configOptions().size(), 1);
    QCOMPARE(client.configOptions().at(0).toMap().value("configId").toString(), QString("model"));
}

void TestAgentprocesOgInitialisering::startfejl_giver_lastError_uden_haeng()
{
    AgentProcess process;
    process.setAgentPath("/definitivt/ikke/eksisterende-agent");
    QElapsedTimer ur;
    ur.start();
    QVERIFY(!process.start());
    QVERIFY(ur.elapsed() < 2000);
    QVERIFY(!process.lastError().isEmpty());
    QVERIFY(!process.isRunning());

    AcpClient client;
    injectTestModels(&client, this);
    client.agentProcess()->setAgentPath("/definitivt/ikke/eksisterende-agent");
    QVERIFY(!client.startAgent());
    QVERIFY(!client.lastError().isEmpty());
}

#include "tst_agentproces_og_initialisering.moc"
