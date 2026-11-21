#include <QtTest/QtTest>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "acpclient.h"
#include "conversationmodel.h"
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
    void messageSent_kun_ved_faktisk_skrevet();
private:
};

REGISTER_TEST(TestPromptOgSvar)

void TestPromptOgSvar::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-prompt");
}

void TestPromptOgSvar::session_new_payload_cwd()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp/arbejde");
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/new"));
    QCOMPARE(o.value("params").toObject().value("cwd").toString(), QString("/tmp/arbejde"));
}

void TestPromptOgSvar::prompt_payload_content_blokke()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "sess-42");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");
    QCOMPARE(client.sessionId(), QString("sess-42"));

    client.sendSessionPrompt("Hej agent");
    QJsonObject o = testJsonObj(client.lastSentForTest());
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
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_prompt_svar.ndjson"));
    QCOMPARE(client.conversationModel()->lastText(), QString("Hej verden"));
}

void TestPromptOgSvar::svar_korreleres_paa_request_id()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");

    client.sendSessionPrompt("test");
    qint64 pid = testIdOf(client.lastSentForTest());
    QVERIFY(client.stopReason().isEmpty());
    QJsonObject stop;
    stop.insert("stopReason", "end_turn");
    client.feedBytesForTest(testResponseJson(pid, stop) + "\n");
    QCOMPARE(client.stopReason(), QString("end_turn"));

    QJsonObject ukendt;
    ukendt.insert("stopReason", "cancelled");
    client.feedBytesForTest(testResponseJson(9999, ukendt) + "\n");
    QCOMPARE(client.stopReason(), QString("end_turn"));
}

void TestPromptOgSvar::end_turn_afslutter_runde()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy afsluttet(&client, SIGNAL(roundFinished()));
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");

    client.sendSessionPrompt("hej");
    QCOMPARE(client.conversationModel()->round(), 1);
    QCOMPARE(client.conversationModel()->status(), QString("kører"));

    client.feedBytesForTest("{\"jsonrpc\":\"2.0\",\"method\":\"end_turn\",\"params\":{\"sessionId\":\"s1\"}}\n");
    QCOMPARE(afsluttet.count(), 1);
    QCOMPARE(client.conversationModel()->status(), QString("klar"));
}

void TestPromptOgSvar::messageSent_kun_ved_faktisk_skrevet()
{
    AcpClient udenEnhed;
    QSignalSpy sendtSpy(&udenEnhed, SIGNAL(messageSent(QByteArray)));
    udenEnhed.sendSessionNew("/tmp");
    QCOMPARE(sendtSpy.count(), 0);

    TestClient client;
    injectTestModels(&client, this);
    QSignalSpy sendtSpy2(&client, SIGNAL(messageSent(QByteArray)));
    client.sendSessionNew("/tmp");
    QCOMPARE(sendtSpy2.count(), 1);
    QVERIFY(!client.lastSentForTest().isEmpty());
}

#include "tst_prompt_og_svar.moc"
