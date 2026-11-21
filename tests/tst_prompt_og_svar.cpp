#include <QtTest/QtTest>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "acpclient.h"
#include "samtalemodel.h"
#include "testregistrering.h"

class TestPromptOgSvar : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void session_new_payload_cwd();
    void prompt_payload_content_blokke();
    void chunk_akkumuleres_til_lastText();
    void svar_korreleres_paa_request_id();
    void end_turn_afslutter_runde();
private:
    QByteArray readFixture(const QString& name);
    qint64 idOf(const QByteArray& json);
    QByteArray responseJson(qint64 id, const QJsonObject& result);
};

REGISTER_TEST(TestPromptOgSvar)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestPromptOgSvar::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + "/skrynja-tst-settings-prompt";
    QDir().mkpath(path);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestPromptOgSvar::readFixture(const QString& name)
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

qint64 TestPromptOgSvar::idOf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestPromptOgSvar::responseJson(qint64 id, const QJsonObject& result)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", result);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestPromptOgSvar::session_new_payload_cwd()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp/arbejde");
    QJsonObject o = jsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/new"));
    QCOMPARE(o.value("params").toObject().value("cwd").toString(), QString("/tmp/arbejde"));
}

void TestPromptOgSvar::prompt_payload_content_blokke()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = idOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "sess-42");
    client.feedBytesForTest(responseJson(id, result) + "\n");
    QCOMPARE(client.sessionId(), QString("sess-42"));

    client.sendSessionPrompt("Hej agent");
    QJsonObject o = jsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/prompt"));
    QJsonObject params = o.value("params").toObject();
    QCOMPARE(params.value("sessionId").toString(), QString("sess-42"));
    QJsonArray prompt = params.value("prompt").toArray();
    QCOMPARE(prompt.size(), 1);
    QCOMPARE(prompt.at(0).toObject().value("type").toString(), QString("text"));
    QCOMPARE(prompt.at(0).toObject().value("text").toString(), QString("Hej agent"));
}

void TestPromptOgSvar::chunk_akkumuleres_til_lastText()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(readFixture("fixture_prompt_svar.ndjson"));
    QCOMPARE(client.conversationModel()->lastText(), QString("Hej verden"));
}

void TestPromptOgSvar::svar_korreleres_paa_request_id()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = idOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(responseJson(id, result) + "\n");

    client.sendSessionPrompt("test");
    qint64 pid = idOf(client.lastSentForTest());
    QVERIFY(client.stopReason().isEmpty());
    QJsonObject stop;
    stop.insert("stopReason", "end_turn");
    client.feedBytesForTest(responseJson(pid, stop) + "\n");
    QCOMPARE(client.stopReason(), QString("end_turn"));

    QJsonObject ukendt;
    ukendt.insert("stopReason", "cancelled");
    client.feedBytesForTest(responseJson(9999, ukendt) + "\n");
    QCOMPARE(client.stopReason(), QString("end_turn"));
}

void TestPromptOgSvar::end_turn_afslutter_runde()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy afsluttet(&client, SIGNAL(roundFinished()));
    client.sendSessionNew("/tmp");
    qint64 id = idOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(responseJson(id, result) + "\n");

    client.sendSessionPrompt("hej");
    QCOMPARE(client.conversationModel()->round(), 1);
    QCOMPARE(client.conversationModel()->status(), QString("kører"));

    client.feedBytesForTest("{\"jsonrpc\":\"2.0\",\"method\":\"end_turn\",\"params\":{\"sessionId\":\"s1\"}}\n");
    QCOMPARE(afsluttet.count(), 1);
    QCOMPARE(client.conversationModel()->status(), QString("klar"));
}

#include "tst_prompt_og_svar.moc"
