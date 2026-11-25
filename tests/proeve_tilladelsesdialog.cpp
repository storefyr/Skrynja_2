#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QVariantList>
#include <QVariantMap>

#include "acpclient.h"
#include "testregistrering.h"
#include "logfile.h"
#include "permissionmodel.h"

class TestTilladelsesdialog : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void anmodning_parses_til_pendingPermission();
    void options_liste_med_optionId();
    void svar_payload_selected_optionId();
    void afvisning_giver_outcome_cancelled();
    void ingen_dobbelt_svar();
    void fejl_paa_prompt_signaleres();
    void ukendt_svar_id_signaleres();
    void permissionRequested_forbundet_i_qml();
    void dialog_tjekker_svar_retvaerdi();
    void clientCapabilities_paa_enhed_ikke_tom();
    void terminal_kald_uden_godkendt_tilladelse_afvises();
    void hemmelighedsfilter_fanger_token_lignende_strenge_i_log();
private:
};

REGISTER_TEST(TestTilladelsesdialog)

void TestTilladelsesdialog::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-tilladelse");
}

void TestTilladelsesdialog::anmodning_parses_til_pendingPermission()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy requested(&client, SIGNAL(permissionRequested()));
    client.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QCOMPARE(requested.count(), 1);
    PermissionModel* model = client.permissionModel();
    QVERIFY(model->isPending());
    QVERIFY(!model->pendingPermission().isEmpty());
    QCOMPARE(model->requestId(), 100);
}

void TestTilladelsesdialog::options_liste_med_optionId()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QVariantMap pending = client.permissionModel()->pendingPermission();
    QVariantList options = pending.value("options").toList();
    QCOMPARE(options.size(), 2);
    QCOMPARE(options.at(0).toMap().value("optionId").toString(), QString("allow"));
    QCOMPARE(options.at(1).toMap().value("optionId").toString(), QString("deny"));
    QCOMPARE(pending.value("toolCall").toMap().value("title").toString(), QString("Skriv fil"));
}

void TestTilladelsesdialog::svar_payload_selected_optionId()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QVERIFY(client.answerPermission("allow"));
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("id").toVariant().toLongLong(), 100);
    QJsonObject outcome = o.value("result").toObject().value("outcome").toObject();
    QCOMPARE(outcome.value("outcome").toString(), QString("selected"));
    QCOMPARE(outcome.value("optionId").toString(), QString("allow"));
}

void TestTilladelsesdialog::afvisning_giver_outcome_cancelled()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QVERIFY(client.declinePermission());
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QJsonObject outcome = o.value("result").toObject().value("outcome").toObject();
    QCOMPARE(outcome.value("outcome").toString(), QString("cancelled"));
}

void TestTilladelsesdialog::ingen_dobbelt_svar()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QCOMPARE(client.permissionModel()->requests(), 1);
    QVERIFY(client.answerPermission("allow"));
    QVERIFY(!client.answerPermission("deny"));
    QVERIFY(!client.declinePermission());
    int responseCount = 0;
    QList<QByteArray> sent = client.sentForTest();
    for (int i = 0; i < sent.size(); ++i) {
        QJsonObject o = testJsonObj(sent.at(i));
        if (o.contains("id") && o.value("id").toVariant().toLongLong() == 100 && o.contains("result"))
            ++responseCount;
    }
    QCOMPARE(responseCount, 1);
}

void TestTilladelsesdialog::fejl_paa_prompt_signaleres()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy fejlSpy(&client, SIGNAL(agentError(QString)));
    client.sendSessionNew("/tmp");
    qint64 newId = testIdOf(client.lastSentForTest());
    QJsonObject newResult;
    newResult.insert("sessionId", "sess-1");
    client.feedBytesForTest(testResponseJson(newId, newResult) + "\n");
    client.sendSessionPrompt("Hej");
    qint64 promptId = testIdOf(client.lastSentForTest());
    client.feedBytesForTest(testErrorResponseJson(promptId, -32000, "prompt afbrudt") + "\n");
    QCOMPARE(client.lastError(), QString("prompt afbrudt"));
    QVERIFY2(fejlSpy.count() >= 1, "agentError ikke emitteret for fejl på session/prompt");
}

void TestTilladelsesdialog::ukendt_svar_id_signaleres()
{
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy fejlSpy(&client, SIGNAL(agentError(QString)));
    client.feedBytesForTest(testResponseJson(999, QJsonObject()) + "\n");
    QVERIFY2(fejlSpy.count() >= 1, "agentError ikke emitteret for ukendt svar-id");
}

void TestTilladelsesdialog::permissionRequested_forbundet_i_qml()
{
    QString qml = testReadSource("qml/harbour-skrynja-2.qml");
    QVERIFY(!qml.isEmpty());
    QVERIFY2(qml.contains("onPermissionRequested"), "permissionRequested ikke forbundet i QML");
    QVERIFY2(qml.contains("onAgentError"), "agentError ikke forbundet i QML");
}

void TestTilladelsesdialog::dialog_tjekker_svar_retvaerdi()
{
    QString qml = testReadSource("qml/pages/TilladelsesDialog.qml");
    QVERIFY(!qml.isEmpty());
    QVERIFY2(qml.contains("if (acp.answerPermission("), "dialog tjekker ikke answerPermission-returværdi");
    QVERIFY2(qml.contains("if (acp.declinePermission("), "dialog tjekker ikke declinePermission-returværdi");
}

void TestTilladelsesdialog::clientCapabilities_paa_enhed_ikke_tom()
{
    QString sessionSrc = testReadSource("kilder/acpsession.cpp");
    QVERIFY2(sessionSrc.contains("terminalAktivPaaEnhed"), "acpsession.cpp skal gate clientCapabilities på terminalAktivPaaEnhed");
    QVERIFY2(testReadSource("kilder/konfiguration.cpp").contains("terminal/aktiv"), "konfiguration.cpp skal eje enhedsindstillingen terminal/aktiv");

    QSettings s;
    s.remove("terminal/aktiv");
    s.sync();
    {
        TestClient client;
        injectTestModels(&client, this);
        QVERIFY(testInitializeParams(client).value("clientCapabilities").toObject().isEmpty());
    }
    s.setValue("terminal/aktiv", true);
    s.sync();
    {
        TestClient client;
        injectTestModels(&client, this);
        QJsonObject cap = testInitializeParams(client).value("clientCapabilities").toObject();
        QVERIFY2(!cap.isEmpty(), "clientCapabilities skal være ikke-tom når terminal/aktiv er sat (på enheden)");
        QVERIFY2(cap.contains("terminal") || cap.contains("fs"), "clientCapabilities skal indeholde terminal/fs på enheden");
    }
    s.remove("terminal/aktiv");
    s.sync();
}

void TestTilladelsesdialog::terminal_kald_uden_godkendt_tilladelse_afvises()
{
    QSettings s;
    s.remove("terminal/aktiv");
    s.sync();

    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "sess-term");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");

    QJsonObject tc;
    tc.insert("toolCallId", "tc-t1");
    tc.insert("title", "k\u00f8r kommando");
    tc.insert("kind", "terminal/exec");
    QJsonObject params;
    params.insert("toolCall", tc);
    QJsonArray options;
    QJsonObject allow;
    allow.insert("optionId", "allow");
    allow.insert("label", "Tillad");
    options.append(allow);
    params.insert("options", options);
    QJsonObject req;
    req.insert("jsonrpc", "2.0");
    req.insert("id", QJsonValue(100));
    req.insert("method", "session/request_permission");
    req.insert("params", params);
    client.feedBytesForTest(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");

    QVERIFY(client.permissionModel()->isPending());
    QVERIFY2(!client.answerPermission("allow"),
             "terminal-kald uden aktiveret terminal-kapacitet må ikke kunne tillades");
    for (int i = 0; i < client.sentForTest().size(); ++i)
        QVERIFY2(!client.sentForTest().at(i).contains("\"allowed\""), "der blev sendt et allow-svar uden kapacitet");

    s.setValue("terminal/aktiv", true);
    s.sync();
    TestClient client2;
    injectTestModels(&client2, this);
    client2.setTestMode(true);
    client2.sendSessionNew("/tmp");
    qint64 id2 = testIdOf(client2.lastSentForTest());
    QJsonObject result2;
    result2.insert("sessionId", "sess-term-2");
    client2.feedBytesForTest(testResponseJson(id2, result2) + "\n");
    client2.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QVERIFY(client2.permissionModel()->isPending());
    QVERIFY(client2.answerPermission("allow"));
    s.remove("terminal/aktiv");
    s.sync();
}

void TestTilladelsesdialog::hemmelighedsfilter_fanger_token_lignende_strenge_i_log()
{
    QString sti = QDir::tempPath() + "/skrynja-tst-hemmelighedsfilter.log";
    QFile::remove(sti);
    LogFile::instance()->setLogPath(sti);
    LogFile::instance()->writeLine("agent svarede med token ghp_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 her");
    LogFile::instance()->writeLine("almindelig loglinje uden hemmeligheder");
    LogFile::instance()->writeLine("Bearer eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiIxMjM0In0.dummysig");
    QFile f(sti);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QString log = QString::fromUtf8(f.readAll());
    f.close();
    QVERIFY2(!log.contains("ghp_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"), "ghp_-token blev logget råt");
    QVERIFY2(!log.contains("eyJhbGciOiJIUzI1NiJ9"), "JWT-lignende token blev logget råt");
    QVERIFY2(log.contains("almindelig loglinje uden hemmeligheder"), "almindelige linjer skal ikke filtreres");
    QFile::remove(sti);
}

#include "proeve_tilladelsesdialog.moc"
