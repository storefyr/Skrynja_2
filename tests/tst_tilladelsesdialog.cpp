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
private:
};

REGISTER_TEST(TestTilladelsesdialog)

void TestTilladelsesdialog::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-tilladelse");
}

void TestTilladelsesdialog::anmodning_parses_til_pendingPermission()
{
    AcpClient client;
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
    AcpClient client;
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
    AcpClient client;
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
    AcpClient client;
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
    AcpClient client;
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
    AcpClient client;
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
    AcpClient client;
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

#include "tst_tilladelsesdialog.moc"
