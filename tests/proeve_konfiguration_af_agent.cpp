#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSignalSpy>
#include <QVariantMap>

#include "acpclient.h"
#include "agentprocess.h"
#include "configurationmodel.h"
#include "konfiguration.h"
#include "sessionstore.h"
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
    void en_config_ejer_med_signaler();
private:
};

REGISTER_TEST(TestKonfigurationAfAgent)

void TestKonfigurationAfAgent::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-konfig");
}

void TestKonfigurationAfAgent::config_options_fra_initialize_svar()
{
    TestClient client;
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
    TestClient client;
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
    TestClient client;
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
    TestClient client;
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

void TestKonfigurationAfAgent::en_config_ejer_med_signaler()
{
    QString agentSrc = testReadSource("src/agentprocess.cpp");
    QString storeSrc = testReadSource("src/sessionstore.cpp");
    QString storeHdr = testReadSource("src/sessionstore.h");
    QString cfgHdr = testReadSource("src/konfiguration.h");
    QString cfgSrc = testReadSource("src/konfiguration.cpp");
    QVERIFY2(!agentSrc.contains("QSettings"), "agentprocess.cpp reads QSettings directly");
    QVERIFY2(!storeSrc.contains("QSettings"), "sessionstore.cpp reads QSettings directly");
    QVERIFY2(!storeHdr.contains("QSettings"), "sessionstore.h still has a QSettings member");
    QVERIFY2(!cfgHdr.isEmpty() && !cfgSrc.isEmpty(), "konfiguration.h/.cpp missing");
    QVERIFY2(cfgHdr.contains("agentPath") && cfgHdr.contains("sessionId"), "config owner lacks agentPath/sessionId");
    QVERIFY2(cfgHdr.contains("agentPathChanged") && cfgHdr.contains("sessionIdChanged"), "config owner lacks change signals");

    QSettings s;
    s.clear();
    s.sync();

    Konfiguration* ejer = Konfiguration::instance();
    QVERIFY(ejer != 0);
    QSignalSpy stiSpy(ejer, SIGNAL(agentPathChanged()));
    QSignalSpy sessionSpy(ejer, SIGNAL(sessionIdChanged()));

    AgentProcess process;
    process.setAgentPath("/tmp/config-ejer-agent");
    QCOMPARE(ejer->agentPath(), QString("/tmp/config-ejer-agent"));
    QCOMPARE(process.agentPath(), QString("/tmp/config-ejer-agent"));
    QCOMPARE(stiSpy.count(), 1);

    SessionStore store;
    QVERIFY(!store.hasSessionId());
    store.saveSessionId("cfg-s1");
    QCOMPARE(ejer->sessionId(), QString("cfg-s1"));
    QCOMPARE(store.sessionId(), QString("cfg-s1"));
    QVERIFY(store.hasSessionId());
    QCOMPARE(sessionSpy.count(), 1);

    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.setAgentsti("/tmp/config-ejer-agent-2");
    QCOMPARE(client.agentPath(), QString("/tmp/config-ejer-agent-2"));
    QCOMPARE(ejer->agentPath(), QString("/tmp/config-ejer-agent-2"));
    QCOMPARE(stiSpy.count(), 2);

    client.sendInitialize();
    client.feedBytesForTest(testReadFixture("fixture_initialize_config.ndjson"));
    QCOMPARE(client.configOptions().size(), 2);

    s.clear();
    s.sync();
}

#include "proeve_konfiguration_af_agent.moc"
