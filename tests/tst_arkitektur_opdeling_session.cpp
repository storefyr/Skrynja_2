#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

#include "acpbesked.h"
#include "acpclient.h"
#include "acpprotokol.h"
#include "acpsession.h"
#include "samtalemodel.h"
#include "testregistrering.h"

Q_DECLARE_METATYPE(AcpMessage)

class TestArkitekturOpdelingSession : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void acp_session_haandterer_initialize_new_load_prompt_cancel();
    void session_id_lagres_i_sessionlager();
    void acpclient_har_ingen_sessionlogik();
    void eksisterende_session_tests_stadig_groenne();
private:
    QString repoRoot() const;
    QString readSource(const QString& rel) const;
    QByteArray readFixture(const QString& name) const;
    QJsonObject jsonObj(const QByteArray& json) const;
    qint64 idOf(const QByteArray& json) const;
    QByteArray responseJson(qint64 id, const QJsonObject& result) const;
    QByteArray errorResponseJson(qint64 id, int kode, const QString& message) const;
};

REGISTER_TEST(TestArkitekturOpdelingSession)

void TestArkitekturOpdelingSession::initTestCase()
{
    qRegisterMetaType<AcpMessage>("AcpMessage");
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + "/skrynja-tst-settings-ark-session";
    QDir().mkpath(path);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings s;
    s.clear();
    s.sync();
}

QString TestArkitekturOpdelingSession::repoRoot() const
{
    QString root = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(root + "/realiseringsmanifest_for_funktioner_og_parrede_tests.json"))
        root = "..";
    return root;
}

QString TestArkitekturOpdelingSession::readSource(const QString& rel) const
{
    QFile f(repoRoot() + "/" + rel);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(f.readAll());
}

QByteArray TestArkitekturOpdelingSession::readFixture(const QString& name) const
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

QJsonObject TestArkitekturOpdelingSession::jsonObj(const QByteArray& json) const
{
    return QJsonDocument::fromJson(json).object();
}

qint64 TestArkitekturOpdelingSession::idOf(const QByteArray& json) const
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestArkitekturOpdelingSession::responseJson(qint64 id, const QJsonObject& result) const
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", result);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

QByteArray TestArkitekturOpdelingSession::errorResponseJson(qint64 id, int kode, const QString& message) const
{
    QJsonObject error;
    error.insert("code", kode);
    error.insert("message", message);
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("error", error);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestArkitekturOpdelingSession::acp_session_haandterer_initialize_new_load_prompt_cancel()
{
    AcpProtokol protokol;
    AcpSession session(&protokol);
    QSignalSpy outSpy(&session, SIGNAL(messageOut(AcpMessage)));
    QSignalSpy createdSpy(&session, SIGNAL(sessionCreated(QString)));
    QSignalSpy historySpy(&session, SIGNAL(historyChanged()));
    QSignalSpy promptSpy(&session, SIGNAL(promptStarted()));

    session.initialize();
    QCOMPARE(outSpy.count(), 1);
    QByteArray initReq = outSpy.at(0).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(initReq).value("method").toString(), QString("initialize"));
    QJsonObject initParams = jsonObj(initReq).value("params").toObject();
    QCOMPARE(initParams.value("protocolVersion").toInt(), 1);
    QVERIFY(initParams.value("clientCapabilities").toObject().isEmpty());

    session.sendSessionNew("/tmp/arbejde");
    QCOMPARE(outSpy.count(), 2);
    QByteArray newReq = outSpy.at(1).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(newReq).value("method").toString(), QString("session/new"));
    QCOMPARE(jsonObj(newReq).value("params").toObject().value("cwd").toString(), QString("/tmp/arbejde"));
    qint64 newId = idOf(newReq);

    session.sendSessionLoad("gammel-session");
    QCOMPARE(outSpy.count(), 3);
    QByteArray loadReq = outSpy.at(2).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(loadReq).value("method").toString(), QString("session/load"));
    QCOMPARE(jsonObj(loadReq).value("params").toObject().value("sessionId").toString(), QString("gammel-session"));
    qint64 loadId = idOf(loadReq);

    session.sendSessionPrompt("Hej agent");
    QCOMPARE(outSpy.count(), 4);
    QCOMPARE(promptSpy.count(), 1);
    QByteArray promptReq = outSpy.at(3).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(promptReq).value("method").toString(), QString("session/prompt"));
    QJsonArray prompt = jsonObj(promptReq).value("params").toObject().value("prompt").toArray();
    QCOMPARE(prompt.size(), 1);
    QCOMPARE(prompt.at(0).toObject().value("type").toString(), QString("text"));
    QCOMPARE(prompt.at(0).toObject().value("text").toString(), QString("Hej agent"));
    qint64 promptId = idOf(promptReq);

    session.sessionCancel();
    QCOMPARE(outSpy.count(), 5);
    QByteArray cancelReq = outSpy.at(4).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(cancelReq).value("method").toString(), QString("session/cancel"));
    QVERIFY(!jsonObj(cancelReq).contains("id"));

    protokol.handleMessage(AcpMessage::errorResponse(loadId, -32602, "ingen session"));
    QCOMPARE(outSpy.count(), 6);
    QByteArray fallback = outSpy.at(5).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(fallback).value("method").toString(), QString("session/new"));
    QVERIFY(!session.hadRestoredHistory());
    QVERIFY(historySpy.count() >= 1);

    QJsonObject newResult;
    newResult.insert("sessionId", "sess-42");
    protokol.handleMessage(AcpMessage::response(newId, newResult));
    QCOMPARE(createdSpy.count(), 1);
    QCOMPARE(createdSpy.at(0).at(0).toString(), QString("sess-42"));
    QCOMPARE(session.sessionId(), QString("sess-42"));
    QVERIFY(!session.hadRestoredHistory());

    QJsonObject promptResult;
    promptResult.insert("stopReason", "end_turn");
    protokol.handleMessage(AcpMessage::response(promptId, promptResult));
    QCOMPARE(session.stopReason(), QString("end_turn"));
}

void TestArkitekturOpdelingSession::session_id_lagres_i_sessionlager()
{
    QSettings s;
    s.clear();
    s.sync();

    AcpProtokol protokol;
    AcpSession session(&protokol);
    QSignalSpy outSpy(&session, SIGNAL(messageOut(AcpMessage)));

    session.sendSessionNew("/tmp");
    qint64 newId = idOf(outSpy.at(0).at(0).value<AcpMessage>().tilJson());
    QJsonObject result;
    result.insert("sessionId", "sess-99");
    protokol.handleMessage(AcpMessage::response(newId, result));

    QSettings check;
    QCOMPARE(check.value("session/id").toString(), QString("sess-99"));
    QCOMPARE(session.sessionId(), QString("sess-99"));

    session.startSession();
    QByteArray last = outSpy.at(outSpy.count() - 1).at(0).value<AcpMessage>().tilJson();
    QCOMPARE(jsonObj(last).value("method").toString(), QString("session/load"));
    QCOMPARE(jsonObj(last).value("params").toObject().value("sessionId").toString(), QString("sess-99"));
    qint64 loadId = idOf(last);
    protokol.handleMessage(AcpMessage::response(loadId, result));
    QVERIFY(session.hadRestoredHistory());
}

void TestArkitekturOpdelingSession::acpclient_har_ingen_sessionlogik()
{
    QString header = readSource("src/acpclient.h");
    QString kilde = readSource("src/acpclient.cpp");
    QString protokolKilde = readSource("src/acpprotokol.cpp");
    QString sessionKilde = readSource("src/acpsession.cpp");
    QVERIFY(!header.isEmpty());
    QVERIFY(!kilde.isEmpty());
    QVERIFY(!sessionKilde.isEmpty());
    QVERIFY(!header.contains("m_sessionId"));
    QVERIFY(!header.contains("m_store"));
    QVERIFY(!header.contains("SessionStore"));
    QVERIFY(!kilde.contains("m_sessionId"));
    QVERIFY(!kilde.contains("m_store"));
    QVERIFY(!kilde.contains("saveSessionId"));
    QVERIFY(!kilde.contains("hasSessionId"));

    QCOMPARE(kilde.count("session/new"), 0);
    QCOMPARE(kilde.count("session/load"), 0);
    QCOMPARE(kilde.count("session/prompt"), 0);
    QCOMPARE(kilde.count("session/cancel"), 0);
    QCOMPARE(protokolKilde.count("session/prompt"), 1);
    QCOMPARE(sessionKilde.count("session/cancel"), 1);
    QCOMPARE(sessionKilde.count("session/new"), 0);
    QCOMPARE(sessionKilde.count("session/load"), 0);
    QCOMPARE(kilde.count("set_config_option"), 1);

    AcpClient client;
    client.setTestMode(true);
    client.sendSessionNew("/tmp/x");
    QCOMPARE(jsonObj(client.lastSentForTest()).value("method").toString(), QString("session/new"));
    client.sendSessionPrompt("Hej");
    QCOMPARE(jsonObj(client.lastSentForTest()).value("method").toString(), QString("session/prompt"));
    QVERIFY(client.sessionId().isEmpty());
}

void TestArkitekturOpdelingSession::eksisterende_session_tests_stadig_groenne()
{
    QSettings s;
    s.clear();
    s.sync();

    AcpClient client;
    client.setTestMode(true);
    QSignalSpy roundSpy(&client, SIGNAL(roundFinished()));

    client.sendInitialize();
    qint64 initId = idOf(client.lastSentForTest());
    QJsonObject initResult;
    initResult.insert("protocolVersion", 1);
    client.feedBytesForTest(responseJson(initId, initResult) + "\n");

    client.startSession();
    QCOMPARE(jsonObj(client.lastSentForTest()).value("method").toString(), QString("session/new"));
    qint64 newId = idOf(client.lastSentForTest());
    QJsonObject newResult;
    newResult.insert("sessionId", "sess-42");
    client.feedBytesForTest(responseJson(newId, newResult) + "\n");
    QCOMPARE(client.sessionId(), QString("sess-42"));

    client.sendSessionPrompt("Hej agent");
    client.feedBytesForTest(readFixture("fixture_prompt_svar.ndjson"));
    QCOMPARE(client.conversationModel()->lastText(), QString("Hej verden"));
    QVERIFY(roundSpy.count() >= 1);

    client.feedBytesForTest(readFixture("fixture_end_turn.ndjson"));
    QVERIFY(roundSpy.count() >= 2);

    QStringList navne;
    for (int i = 0; i < testRegistrations().size(); ++i)
        navne.append(testRegistrations().at(i).name);
    QVERIFY(navne.contains("TestGenoptagelseAfSamtale"));
    QVERIFY(navne.contains("TestPromptOgSvar"));
    QVERIFY(navne.contains("TestArkitekturOpdelingSession"));
}

#include "tst_arkitektur_opdeling_session.moc"
