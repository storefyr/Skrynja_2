#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "acpclient.h"
#include "selvtest.h"
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
private:
    QByteArray læsFixture(const QString& navn);
    qint64 idAf(const QByteArray& json);
    QByteArray svarJson(qint64 id, const QJsonObject& resultat);
    QString verdictSti() const;
    void kørKædeTilSession(AcpClient& klient);
};

REGISTRER_TEST(TestSelvtestMedVerdict)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestSelvtestMedVerdict::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-selvtest";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
    QDir(QDir::tempPath() + "/skrynja-tst-selvtest").mkpath(".");
}

QString TestSelvtestMedVerdict::verdictSti() const
{
    return QDir::tempPath() + "/skrynja-tst-selvtest/verdict.json";
}

QByteArray TestSelvtestMedVerdict::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

qint64 TestSelvtestMedVerdict::idAf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestSelvtestMedVerdict::svarJson(qint64 id, const QJsonObject& resultat)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", resultat);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestSelvtestMedVerdict::kørKædeTilSession(AcpClient& klient)
{
    QJsonObject initResultat;
    initResultat.insert("protocolVersion", 1);
    initResultat.insert("configOptions", QJsonArray());
    qint64 initId = idAf(klient.sendteForTest().isEmpty() ? QByteArray() : klient.sendteForTest().first());
    klient.feedBytesForTest(svarJson(initId, initResultat) + "\n");
    klient.startSession();
    qint64 newId = idAf(klient.sidsteSendtForTest());
    QJsonObject sessionResultat;
    sessionResultat.insert("sessionId", "s");
    klient.feedBytesForTest(svarJson(newId, sessionResultat) + "\n");
}

void TestSelvtestMedVerdict::verdict_pass_ved_end_turn_og_exit_0()
{
    QFile::remove(verdictSti());
    AcpClient klient;
    klient.sætTestTilstand(true);
    Selvtest selvtest(&klient);
    QSignalSpy færdig(&selvtest, SIGNAL(faerdig(int)));
    selvtest.start("selvtest-prompt", verdictSti());
    kørKædeTilSession(klient);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse_og_svar.ndjson"));
    klient.feedBytesForTest(læsFixture("fixture_end_turn.ndjson"));
    QCOMPARE(færdig.count(), 1);
    QCOMPARE(færdig.at(0).at(0).toInt(), 0);
    QVERIFY(selvtest.verdictSkrevet());
    QCOMPARE(selvtest.verdict().value("resultat").toString(), QString("pass"));
}

void TestSelvtestMedVerdict::verdict_fail_uden_toolcall()
{
    QFile::remove(verdictSti());
    AcpClient klient;
    klient.sætTestTilstand(true);
    Selvtest selvtest(&klient);
    QSignalSpy færdig(&selvtest, SIGNAL(faerdig(int)));
    selvtest.start("selvtest-prompt", verdictSti());
    kørKædeTilSession(klient);
    klient.feedBytesForTest(læsFixture("fixture_end_turn.ndjson"));
    QCOMPARE(færdig.count(), 1);
    QCOMPARE(færdig.at(0).at(0).toInt(), 1);
    QCOMPARE(selvtest.verdict().value("resultat").toString(), QString("fail"));
}

void TestSelvtestMedVerdict::ingen_dobbelt_verdict()
{
    QFile::remove(verdictSti());
    AcpClient klient;
    klient.sætTestTilstand(true);
    Selvtest selvtest(&klient);
    QSignalSpy færdig(&selvtest, SIGNAL(faerdig(int)));
    selvtest.start("selvtest-prompt", verdictSti());
    kørKædeTilSession(klient);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse_og_svar.ndjson"));
    klient.feedBytesForTest(læsFixture("fixture_end_turn.ndjson"));
    QCOMPARE(færdig.count(), 1);
    QFile f(verdictSti());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QByteArray før = f.readAll();
    f.close();
    klient.feedBytesForTest(læsFixture("fixture_end_turn.ndjson"));
    klient.feedBytesForTest(læsFixture("fixture_tilladelse_og_svar.ndjson"));
    QCOMPARE(færdig.count(), 1);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(f.readAll()), QString::fromUtf8(før));
    f.close();
}

void TestSelvtestMedVerdict::verdict_ved_agentdoed()
{
    QFile::remove(verdictSti());
    AcpClient klient;
    klient.sætTestTilstand(true);
    Selvtest selvtest(&klient);
    QSignalSpy færdig(&selvtest, SIGNAL(faerdig(int)));
    selvtest.start("selvtest-prompt", verdictSti());
    klient.simulerAgentDødForTest(1);
    QCOMPARE(færdig.count(), 1);
    QCOMPARE(færdig.at(0).at(0).toInt(), 1);
    QCOMPARE(selvtest.verdict().value("resultat").toString(), QString("fail"));
    QVERIFY(selvtest.verdict().value("årsag").toString().contains("agentdød"));
}

void TestSelvtestMedVerdict::verdict_ved_startfejl()
{
    QFile::remove(verdictSti());
    QSettings s;
    s.clear();
    s.sync();
    AcpClient klient;
    klient.setAgentsti("/definitivt/ikke/eksisterende-agent-selvtest");
    Selvtest selvtest(&klient);
    QSignalSpy færdig(&selvtest, SIGNAL(faerdig(int)));
    selvtest.start("selvtest-prompt", verdictSti());
    QCOMPARE(færdig.count(), 1);
    QCOMPARE(færdig.at(0).at(0).toInt(), 1);
    QVERIFY(selvtest.verdictSkrevet());
    QCOMPARE(selvtest.verdict().value("resultat").toString(), QString("fail"));
    s.remove("agent/sti");
    s.sync();
}

void TestSelvtestMedVerdict::verdict_fil_skrives_og_laeses_tilbage()
{
    QFile::remove(verdictSti());
    AcpClient klient;
    klient.sætTestTilstand(true);
    Selvtest selvtest(&klient);
    QSignalSpy færdig(&selvtest, SIGNAL(faerdig(int)));
    selvtest.start("selvtest-prompt", verdictSti());
    kørKædeTilSession(klient);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse_og_svar.ndjson"));
    klient.feedBytesForTest(læsFixture("fixture_end_turn.ndjson"));
    QCOMPARE(færdig.count(), 1);
    QVERIFY(QFileInfo::exists(verdictSti()));
    QFile f(verdictSti());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QJsonObject fraFil = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    QCOMPARE(fraFil.value("resultat").toString(), QString("pass"));
    QVERIFY(fraFil.contains("årsag"));
    QVERIFY(fraFil.contains("tidsstempel"));
    QCOMPARE(selvtest.verdict().value("resultat").toString(), fraFil.value("resultat").toString());
}

#include "tst_selvtest_med_verdict.moc"
