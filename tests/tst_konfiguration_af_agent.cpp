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
    QByteArray læsFixture(const QString& navn);
    qint64 idAf(const QByteArray& json);
    QByteArray svarJson(qint64 id, const QJsonObject& resultat);
};

REGISTRER_TEST(TestKonfigurationAfAgent)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestKonfigurationAfAgent::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-konfig";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestKonfigurationAfAgent::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

qint64 TestKonfigurationAfAgent::idAf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestKonfigurationAfAgent::svarJson(qint64 id, const QJsonObject& resultat)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", resultat);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestKonfigurationAfAgent::config_options_fra_initialize_svar()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendInitialize();
    klient.feedBytesForTest(læsFixture("fixture_initialize_config.ndjson"));
    Konfigurationsmodel* model = klient.konfigurationsmodel();
    QCOMPARE(model->configOptions().size(), 2);
    QCOMPARE(model->findConfig("model").value("value").toString(), QString("vibe-large"));
    QCOMPARE(model->findConfig("maxTokens").value("value").toInt(), 4096);
    QCOMPARE(klient.configOptions().size(), 2);
}

void TestKonfigurationAfAgent::set_config_option_payload()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendSessionNew("/tmp");
    qint64 id = idAf(klient.sidsteSendtForTest());
    QJsonObject resultat;
    resultat.insert("sessionId", "s1");
    klient.feedBytesForTest(svarJson(id, resultat) + "\n");

    klient.setConfigOption("model", "vibe-x");
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
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
    AcpClient klient;
    klient.setAgentsti("/tmp/min-agent");
    QCOMPARE(klient.agentsti(), QString("/tmp/min-agent"));
    AgentProces proces;
    QCOMPARE(proces.agentsti(), QString("/tmp/min-agent"));
    s.remove("agent/sti");
    s.sync();
}

void TestKonfigurationAfAgent::standard_agentsti_er_vibe_acp()
{
    QSettings s;
    s.remove("agent/sti");
    s.sync();
    AgentProces proces;
    QCOMPARE(proces.agentsti(), QString("/home/defaultuser/.local/bin/vibe-acp"));
}

void TestKonfigurationAfAgent::clientCapabilities_fortsat_tom()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendInitialize();
    QJsonObject params = jsonObj(klient.sidsteSendtForTest()).value("params").toObject();
    QJsonValue cap = params.value("clientCapabilities");
    QVERIFY(cap.isObject());
    QVERIFY(cap.toObject().isEmpty());
}

#include "tst_konfiguration_af_agent.moc"
