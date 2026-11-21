#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "acpclient.h"
#include "selftest.h"
#include "testregistrering.h"

class TestSelvtestMedVerdict : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void verdict_pass_ved_end_turn_og_exit_0();
    void verdict_fail_uden_toolcall();
    void ingen_dobbelt_verdict();
    void verdict_ved_agentdoed();
    void verdict_ved_startfejl();
    void verdict_fil_skrives_og_laeses_tilbage();
    void watchdog_udloeser_verdict_fail_ved_haeng();
private:
    QString verdictPath() const;
    void runChainToSession(AcpClient& client);
};

REGISTER_TEST(TestSelvtestMedVerdict)

void TestSelvtestMedVerdict::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-selvtest");
    QDir(QDir::tempPath() + "/skrynja-tst-selvtest").mkpath(".");
}

QString TestSelvtestMedVerdict::verdictPath() const
{
    return QDir::tempPath() + "/skrynja-tst-selvtest/verdict.json";
}

void TestSelvtestMedVerdict::runChainToSession(AcpClient& client)
{
    QJsonObject initResult;
    initResult.insert("protocolVersion", 1);
    initResult.insert("configOptions", QJsonArray());
    qint64 initId = testIdOf(client.sentForTest().isEmpty() ? QByteArray() : client.sentForTest().first());
    client.feedBytesForTest(testResponseJson(initId, initResult) + "\n");
    client.startSession();
    qint64 newId = testIdOf(client.lastSentForTest());
    QJsonObject sessionResult;
    sessionResult.insert("sessionId", "s");
    client.feedBytesForTest(testResponseJson(newId, sessionResult) + "\n");
}

void TestSelvtestMedVerdict::verdict_pass_ved_end_turn_og_exit_0()
{
    QFile::remove(verdictPath());
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    SelfTest selfTest(&client);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("selvtest-prompt", verdictPath());
    runChainToSession(client);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse_og_svar.ndjson"));
    client.feedBytesForTest(testReadFixture("fixture_end_turn.ndjson"));
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.at(0).at(0).toInt(), 0);
    QVERIFY(selfTest.verdictWritten());
    QCOMPARE(selfTest.verdict().value("resultat").toString(), QString("pass"));
}

void TestSelvtestMedVerdict::verdict_fail_uden_toolcall()
{
    QFile::remove(verdictPath());
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    SelfTest selfTest(&client);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("selvtest-prompt", verdictPath());
    runChainToSession(client);
    client.feedBytesForTest(testReadFixture("fixture_end_turn.ndjson"));
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.at(0).at(0).toInt(), 1);
    QCOMPARE(selfTest.verdict().value("resultat").toString(), QString("fail"));
}

void TestSelvtestMedVerdict::ingen_dobbelt_verdict()
{
    QFile::remove(verdictPath());
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    SelfTest selfTest(&client);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("selvtest-prompt", verdictPath());
    runChainToSession(client);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse_og_svar.ndjson"));
    client.feedBytesForTest(testReadFixture("fixture_end_turn.ndjson"));
    QCOMPARE(finished.count(), 1);
    QFile f(verdictPath());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QByteArray before = f.readAll();
    f.close();
    client.feedBytesForTest(testReadFixture("fixture_end_turn.ndjson"));
    client.feedBytesForTest(testReadFixture("fixture_tilladelse_og_svar.ndjson"));
    QCOMPARE(finished.count(), 1);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(f.readAll()), QString::fromUtf8(before));
    f.close();
}

void TestSelvtestMedVerdict::verdict_ved_agentdoed()
{
    QFile::remove(verdictPath());
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    SelfTest selfTest(&client);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("selvtest-prompt", verdictPath());
    client.simulateAgentDeathForTest(1);
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.at(0).at(0).toInt(), 1);
    QCOMPARE(selfTest.verdict().value("resultat").toString(), QString("fail"));
    QVERIFY(selfTest.verdict().value("årsag").toString().contains("agentdød"));
}

void TestSelvtestMedVerdict::verdict_ved_startfejl()
{
    QFile::remove(verdictPath());
    QSettings s;
    s.clear();
    s.sync();
    AcpClient client;
    injectTestModels(&client, this);
    client.setAgentsti("/definitivt/ikke/eksisterende-agent-selvtest");
    SelfTest selfTest(&client);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("selvtest-prompt", verdictPath());
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.at(0).at(0).toInt(), 1);
    QVERIFY(selfTest.verdictWritten());
    QCOMPARE(selfTest.verdict().value("resultat").toString(), QString("fail"));
    s.remove("agent/sti");
    s.sync();
}

void TestSelvtestMedVerdict::verdict_fil_skrives_og_laeses_tilbage()
{
    QFile::remove(verdictPath());
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    SelfTest selfTest(&client);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("selvtest-prompt", verdictPath());
    runChainToSession(client);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse_og_svar.ndjson"));
    client.feedBytesForTest(testReadFixture("fixture_end_turn.ndjson"));
    QCOMPARE(finished.count(), 1);
    QVERIFY(QFileInfo::exists(verdictPath()));
    QFile f(verdictPath());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QJsonObject fromFile = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    QCOMPARE(fromFile.value("resultat").toString(), QString("pass"));
    QVERIFY(fromFile.contains("årsag"));
    QVERIFY(fromFile.contains("tidsstempel"));
    QCOMPARE(selfTest.verdict().value("resultat").toString(), fromFile.value("resultat").toString());
}

void TestSelvtestMedVerdict::watchdog_udloeser_verdict_fail_ved_haeng()
{
    QFile::remove(verdictPath());
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    SelfTest selfTest(&client);
    selfTest.setWatchdogMsForTest(50);
    QSignalSpy finished(&selfTest, SIGNAL(finished(int)));
    selfTest.start("prompt-uden-svar", verdictPath());
    QTest::qWait(500);
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.at(0).at(0).toInt(), 1);
    QVERIFY(selfTest.verdictWritten());
    QCOMPARE(selfTest.verdict().value("resultat").toString(), QString("fail"));
}

#include "tst_selvtest_med_verdict.moc"
