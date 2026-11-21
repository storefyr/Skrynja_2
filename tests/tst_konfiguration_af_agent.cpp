#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QVariantMap>

#include "acpclient.h"
#include "agentprocess.h"
#include "configurationmodel.h"
#include "testregistrering.h"

class TestKonfigurationAfAgent : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void config_options_fra_initialize_svar();
    void set_config_option_payload();
    void agentsti_override_fra_settings();
    void standard_agentsti_er_vibe_acp();
    void clientCapabilities_fortsat_tom();
    void set_config_option_debounced();
private:
};

REGISTER_TEST(TestKonfigurationAfAgent)

void TestKonfigurationAfAgent::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-konfig");
}

void TestKonfigurationAfAgent::config_options_fra_initialize_svar()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendInitialize();
    client.feedBytesForTest(testReadFixture("fixture_initialize_config.ndjson"));
    ConfigurationModel* model = client.configurationModel();
    QCOMPARE(model->configOptions().size(), 2);
    QCOMPARE(model->findConfig("model").value("value").toString(), QString("vibe-large"));
    QCOMPARE(model->findConfig("maxTokens").value("value").toInt(), 4096);
    QCOMPARE(client.configOptions().size(), 2);
}

void TestKonfigurationAfAgent::set_config_option_payload()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");

    client.setConfigOption("model", "vibe-x");
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/set_config_option"));
    QJsonObject params = o.value("params").toObject();
    QCOMPARE(params.value("configId").toString(), QString("model"));
    QCOMPARE(params.value("value").toString(), QString("vibe-x"));
    QCOMPARE(params.value("sessionId").toString(), QString("s1"));
}

void TestKonfigurationAfAgent::agentsti_override_fra_settings()
{
    QSettings s;
    s.clear();
    s.sync();
    AcpClient client;
    injectTestModels(&client, this);
    client.setAgentsti("/tmp/min-agent");
    QCOMPARE(client.agentPath(), QString("/tmp/min-agent"));
    AgentProcess process;
    QCOMPARE(process.agentPath(), QString("/tmp/min-agent"));
    s.remove("agent/sti");
    s.sync();
}

void TestKonfigurationAfAgent::standard_agentsti_er_vibe_acp()
{
    QCOMPARE(testStandardAgentsti(), QString("/home/defaultuser/.local/bin/vibe-acp"));
}

void TestKonfigurationAfAgent::clientCapabilities_fortsat_tom()
{
    AcpClient client;
    injectTestModels(&client, this);
    QJsonValue cap = testInitializeParams(client).value("clientCapabilities");
    QVERIFY(cap.isObject());
    QVERIFY(cap.toObject().isEmpty());
}

void TestKonfigurationAfAgent::set_config_option_debounced()
{
    QString qml = testReadSource("qml/pages/KonfigurationsSide.qml");
    QVERIFY(!qml.isEmpty());
    QVERIFY2(qml.contains("Timer"), "ingen debounce-Timer i KonfigurationsSide");
    QVERIFY2(qml.contains("restart()"), "onTextChanged kalder ikke timer.restart()");
    QVERIFY2(qml.contains("onTriggered: acp.setConfigOption"), "setConfigOption ligger ikke i onTriggered");
    QVERIFY2(!qml.contains("onTextChanged: acp.setConfigOption"), "onTextChanged sender stadig pr. tastetryk");
}

#include "tst_konfiguration_af_agent.moc"
