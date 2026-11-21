#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QVariantMap>

#include "acpclient.h"
#include "agentproces.h"
#include "konfigurationsmodel.h"
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
private:
    QByteArray readFixture(const QString& name);
    qint64 idOf(const QByteArray& json);
    QByteArray responseJson(qint64 id, const QJsonObject& result);
};

REGISTER_TEST(TestKonfigurationAfAgent)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestKonfigurationAfAgent::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + "/skrynja-tst-settings-konfig";
    QDir().mkpath(path);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestKonfigurationAfAgent::readFixture(const QString& name)
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

qint64 TestKonfigurationAfAgent::idOf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestKonfigurationAfAgent::responseJson(qint64 id, const QJsonObject& result)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", result);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestKonfigurationAfAgent::config_options_fra_initialize_svar()
{
    AcpClient client;
    client.setTestMode(true);
    client.sendInitialize();
    client.feedBytesForTest(readFixture("fixture_initialize_config.ndjson"));
    ConfigurationModel* model = client.configurationModel();
    QCOMPARE(model->configOptions().size(), 2);
    QCOMPARE(model->findConfig("model").value("value").toString(), QString("vibe-large"));
    QCOMPARE(model->findConfig("maxTokens").value("value").toInt(), 4096);
    QCOMPARE(client.configOptions().size(), 2);
}

void TestKonfigurationAfAgent::set_config_option_payload()
{
    AcpClient client;
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = idOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(responseJson(id, result) + "\n");

    client.setConfigOption("model", "vibe-x");
    QJsonObject o = jsonObj(client.lastSentForTest());
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
    client.setAgentsti("/tmp/min-agent");
    QCOMPARE(client.agentPath(), QString("/tmp/min-agent"));
    AgentProcess process;
    QCOMPARE(process.agentPath(), QString("/tmp/min-agent"));
    s.remove("agent/sti");
    s.sync();
}

void TestKonfigurationAfAgent::standard_agentsti_er_vibe_acp()
{
    QSettings s;
    s.remove("agent/sti");
    s.sync();
    AgentProcess process;
    QCOMPARE(process.agentPath(), QString("/home/defaultuser/.local/bin/vibe-acp"));
}

void TestKonfigurationAfAgent::clientCapabilities_fortsat_tom()
{
    AcpClient client;
    client.setTestMode(true);
    client.sendInitialize();
    QJsonObject params = jsonObj(client.lastSentForTest()).value("params").toObject();
    QJsonValue cap = params.value("clientCapabilities");
    QVERIFY(cap.isObject());
    QVERIFY(cap.toObject().isEmpty());
}

#include "tst_konfiguration_af_agent.moc"
