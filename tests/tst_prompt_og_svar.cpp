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
    QByteArray læsFixture(const QString& navn);
    qint64 idAf(const QByteArray& json);
    QByteArray svarJson(qint64 id, const QJsonObject& resultat);
};

REGISTRER_TEST(TestPromptOgSvar)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestPromptOgSvar::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-prompt";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestPromptOgSvar::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

qint64 TestPromptOgSvar::idAf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestPromptOgSvar::svarJson(qint64 id, const QJsonObject& resultat)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", resultat);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestPromptOgSvar::session_new_payload_cwd()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendSessionNew("/tmp/arbejde");
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QCOMPARE(o.value("method").toString(), QString("session/new"));
    QCOMPARE(o.value("params").toObject().value("cwd").toString(), QString("/tmp/arbejde"));
}

void TestPromptOgSvar::prompt_payload_content_blokke()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendSessionNew("/tmp");
    qint64 id = idAf(klient.sidsteSendtForTest());
    QJsonObject resultat;
    resultat.insert("sessionId", "sess-42");
    klient.feedBytesForTest(svarJson(id, resultat) + "\n");
    QCOMPARE(klient.sessionId(), QString("sess-42"));

    klient.sendSessionPrompt("Hej agent");
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
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
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_prompt_svar.ndjson"));
    QCOMPARE(klient.samtaleModel()->lastText(), QString("Hej verden"));
}

void TestPromptOgSvar::svar_korreleres_paa_request_id()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.sendSessionNew("/tmp");
    qint64 id = idAf(klient.sidsteSendtForTest());
    QJsonObject resultat;
    resultat.insert("sessionId", "s1");
    klient.feedBytesForTest(svarJson(id, resultat) + "\n");

    klient.sendSessionPrompt("test");
    qint64 pid = idAf(klient.sidsteSendtForTest());
    QVERIFY(klient.stopReason().isEmpty());
    QJsonObject stop;
    stop.insert("stopReason", "end_turn");
    klient.feedBytesForTest(svarJson(pid, stop) + "\n");
    QCOMPARE(klient.stopReason(), QString("end_turn"));

    QJsonObject ukendt;
    ukendt.insert("stopReason", "cancelled");
    klient.feedBytesForTest(svarJson(9999, ukendt) + "\n");
    QCOMPARE(klient.stopReason(), QString("end_turn"));
}

void TestPromptOgSvar::end_turn_afslutter_runde()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    QSignalSpy afsluttet(&klient, SIGNAL(rundeAfsluttet()));
    klient.sendSessionNew("/tmp");
    qint64 id = idAf(klient.sidsteSendtForTest());
    QJsonObject resultat;
    resultat.insert("sessionId", "s1");
    klient.feedBytesForTest(svarJson(id, resultat) + "\n");

    klient.sendSessionPrompt("hej");
    QCOMPARE(klient.samtaleModel()->round(), 1);
    QCOMPARE(klient.samtaleModel()->status(), QString("kører"));

    klient.feedBytesForTest("{\"jsonrpc\":\"2.0\",\"method\":\"end_turn\",\"params\":{\"sessionId\":\"s1\"}}\n");
    QCOMPARE(afsluttet.count(), 1);
    QCOMPARE(klient.samtaleModel()->status(), QString("klar"));
}

#include "tst_prompt_og_svar.moc"
