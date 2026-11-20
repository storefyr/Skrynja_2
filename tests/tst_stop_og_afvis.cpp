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
#include "agentproces.h"
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
    QByteArray læsFixture(const QString& navn);
    qint64 idAf(const QByteArray& json);
    QByteArray svarJson(qint64 id, const QJsonObject& resultat);
};

REGISTRER_TEST(TestStopOgAfvis)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestStopOgAfvis::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-stop";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestStopOgAfvis::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

qint64 TestStopOgAfvis::idAf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestStopOgAfvis::svarJson(qint64 id, const QJsonObject& resultat)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", resultat);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestStopOgAfvis::cancel_notification_payload()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendSessionNew("/tmp");
    qint64 id = idAf(klient.sidsteSendtForTest());
    QJsonObject resultat;
    resultat.insert("sessionId", "s1");
    klient.feedBytesForTest(svarJson(id, resultat) + "\n");

    klient.sessionCancel();
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QCOMPARE(o.value("method").toString(), QString("session/cancel"));
    QVERIFY(!o.contains("id"));
    QCOMPARE(o.value("params").toObject().value("sessionId").toString(), QString("s1"));
}

void TestStopOgAfvis::afvisning_outcome_cancelled()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse.ndjson"));
    QVERIFY(klient.afvisPermission());
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QCOMPARE(o.value("result").toObject().value("outcome").toObject().value("outcome").toString(),
             QString("cancelled"));
}

void TestStopOgAfvis::stopAgent_terminerer_proces()
{
    QVERIFY(QFileInfo::exists("/bin/sleep"));
    AgentProces proces;
    proces.sætProgramOgArgumenterForTest("/bin/sleep", QStringList() << "30");
    QVERIFY(proces.start());
    QVERIFY(proces.kører());
    QElapsedTimer ur;
    ur.start();
    proces.stopAgent();
    QVERIFY(ur.elapsed() < 5000);
    QVERIFY(!proces.kører());
}

void TestStopOgAfvis::ingen_haengende_proces_efter_stop()
{
    AgentProces proces;
    proces.sætProgramOgArgumenterForTest("/bin/sleep", QStringList() << "30");
    QVERIFY(proces.start());
    QVERIFY(proces.kører());
    proces.stopAgent();
    QCOMPARE(proces.proces()->state(), QProcess::NotRunning);
    QCOMPARE(proces.proces()->pid(), 0);
}

void TestStopOgAfvis::stop_og_afvis_er_obligatorisk_i_raekkefoelge()
{
    QString rod = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(rod + "/tjek_manifest_parrede_tests.py"))
        rod = "..";
    QVERIFY2(QFileInfo::exists(rod + "/tjek_manifest_parrede_tests.py"), qPrintable(rod));
    QProcess p;
    p.setWorkingDirectory(rod);
    p.start("python3", QStringList() << "tjek_manifest_parrede_tests.py");
    QVERIFY(p.waitForFinished(15000));
    QString ud = QString::fromUtf8(p.readAllStandardOutput()) + QString::fromUtf8(p.readAllStandardError());
    QCOMPARE(p.exitCode(), 0);
    QVERIFY2(ud.contains("stop_og_afvis ikke sprunget over"), qPrintable(ud));
}

#include "tst_stop_og_afvis.moc"
