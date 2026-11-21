#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

#include "acpclient.h"
#include "agentprocess.h"
#include "testregistrering.h"

class TestStopOgAfvis : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cancel_notification_payload();
    void afvisning_outcome_cancelled();
    void stopAgent_terminerer_proces();
    void ingen_haengende_proces_efter_stop();
    void stop_og_afvis_er_obligatorisk_i_raekkefoelge();
private:
};

REGISTER_TEST(TestStopOgAfvis)

void TestStopOgAfvis::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-stop");
}

void TestStopOgAfvis::cancel_notification_payload()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.sendSessionNew("/tmp");
    qint64 id = testIdOf(client.lastSentForTest());
    QJsonObject result;
    result.insert("sessionId", "s1");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");

    client.sessionCancel();
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/cancel"));
    QVERIFY(!o.contains("id"));
    QCOMPARE(o.value("params").toObject().value("sessionId").toString(), QString("s1"));
}

void TestStopOgAfvis::afvisning_outcome_cancelled()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_tilladelse.ndjson"));
    QVERIFY(client.declinePermission());
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("result").toObject().value("outcome").toObject().value("outcome").toString(),
             QString("cancelled"));
}

void TestStopOgAfvis::stopAgent_terminerer_proces()
{
    QVERIFY(QFileInfo::exists("/bin/sleep"));
    AgentProcess process;
    process.setProgramAndArgsForTest("/bin/sleep", QStringList() << "30");
    QVERIFY(process.start());
    QVERIFY(process.isRunning());
    QElapsedTimer ur;
    ur.start();
    process.stopAgent();
    QVERIFY(ur.elapsed() < 5000);
    QVERIFY(!process.isRunning());
}

void TestStopOgAfvis::ingen_haengende_proces_efter_stop()
{
    AgentProcess process;
    process.setProgramAndArgsForTest("/bin/sleep", QStringList() << "30");
    QVERIFY(process.start());
    QVERIFY(process.isRunning());
    process.stopAgent();
    QCOMPARE(process.process()->state(), QProcess::NotRunning);
    QCOMPARE(process.process()->pid(), 0);
}

void TestStopOgAfvis::stop_og_afvis_er_obligatorisk_i_raekkefoelge()
{
    QString root = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(root + "/tjek_manifest_parrede_tests.py"))
        root = "..";
    QVERIFY2(QFileInfo::exists(root + "/tjek_manifest_parrede_tests.py"), qPrintable(root));
    QProcess p;
    p.setWorkingDirectory(root);
    p.start("python3", QStringList() << "tjek_manifest_parrede_tests.py");
    QVERIFY(p.waitForFinished(15000));
    QString output = QString::fromUtf8(p.readAllStandardOutput()) + QString::fromUtf8(p.readAllStandardError());
    QCOMPARE(p.exitCode(), 0);
    QVERIFY2(output.contains("stop_og_afvis ikke sprunget over"), qPrintable(output));
}

#include "tst_stop_og_afvis.moc"
