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
    void skraerbillede_sendes_som_billedblok_i_session_prompt();
    void clientCapabilities_stadig_tom_indtil_funktion_25();
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

void TestPromptOgSvar::skraerbillede_sendes_som_billedblok_i_session_prompt()
{
    QString sessionSrc = testReadSource("src/acpsession.cpp");
    QVERIFY2(sessionSrc.contains("\"image\""), "acpsession.cpp bygger ikke et image-blok i prompt-payloaden");
    QVERIFY2(sessionSrc.contains("mimeType") && sessionSrc.contains("image/png"), "image-blok mangler mimeType image/png");
    QVERIFY2(sessionSrc.contains("toBase64"), "image-blok skal sendes base64-kodet");

    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "sess-billede");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");

    QByteArray png("\x89PNG\r\n\x1a\n", 8);
    bool kaldt = QMetaObject::invokeMethod(&client, "sendSessionPromptMedBillede",
                                           Q_ARG(QString, QString("se her")), Q_ARG(QByteArray, png));
    QVERIFY2(kaldt, "AcpClient mangler Q_INVOKABLE sendSessionPromptMedBillede");

    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/prompt"));
    QJsonArray blokke = o.value("params").toObject().value("prompt").toArray();
    bool harTekst = false;
    bool harBillede = false;
    QByteArray billedeData;
    for (int i = 0; i < blokke.size(); ++i) {
        QJsonObject b = blokke.at(i).toObject();
        if (b.value("type").toString() == "text" && b.value("text").toString() == "se her")
            harTekst = true;
        if (b.value("type").toString() == "image") {
            QCOMPARE(b.value("mimeType").toString(), QString("image/png"));
            billedeData = QByteArray::fromBase64(b.value("data").toString().toLatin1());
            harBillede = true;
        }
    }
    QVERIFY(harTekst);
    QVERIFY(harBillede);
    QCOMPARE(billedeData, png);

    client.feedBytesForTest(testReadFixture("fixture_prompt_svar.ndjson"));
    QVERIFY(client.conversationModel()->lastText().size() > 0);
}

void TestPromptOgSvar::clientCapabilities_stadig_tom_indtil_funktion_25()
{
    QSettings s;
    s.remove("terminal/aktiv");
    s.sync();
    TestClient client;
    injectTestModels(&client, this);
    QJsonValue cap = testInitializeParams(client).value("clientCapabilities");
    QVERIFY(cap.isObject());
    QVERIFY2(cap.toObject().isEmpty(), "clientCapabilities skal være tom i værtsløkken indtil funktion 25");
}

#include "proeve_prompt_og_svar.moc"
