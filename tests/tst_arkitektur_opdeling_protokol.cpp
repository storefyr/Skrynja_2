#include <QtTest/QtTest>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "acpbesked.h"
#include "acpclient.h"
#include "acpframereader.h"
#include "acpprotokol.h"
#include "testregistrering.h"

Q_DECLARE_METATYPE(AcpMessage)

class TestArkitekturOpdelingProtokol : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void acp_protokol_haandterer_dispatch();
    void request_id_bogfoering_i_acp_protokol();
    void acpclient_har_ingen_dispatch();
    void eksisterende_rygrad_tests_stadig_groenne();
private:
    QString repoRoot() const;
    QString readSource(const QString& rel) const;
    QByteArray readFixture(const QString& name) const;
};

REGISTER_TEST(TestArkitekturOpdelingProtokol)

void TestArkitekturOpdelingProtokol::initTestCase()
{
    qRegisterMetaType<AcpMessage>("AcpMessage");
}

QString TestArkitekturOpdelingProtokol::repoRoot() const
{
    QString root = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(root + "/realiseringsmanifest_for_funktioner_og_parrede_tests.json"))
        root = "..";
    return root;
}

QString TestArkitekturOpdelingProtokol::readSource(const QString& rel) const
{
    QFile f(repoRoot() + "/" + rel);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(f.readAll());
}

QByteArray TestArkitekturOpdelingProtokol::readFixture(const QString& name) const
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestArkitekturOpdelingProtokol::acp_protokol_haandterer_dispatch()
{
    AcpProtokol protokol;
    QSignalSpy initSpy(&protokol, SIGNAL(initializeResponded(QJsonObject)));
    QSignalSpy errSpy(&protokol, SIGNAL(responseError(QString,QString)));
    QSignalSpy loadFailSpy(&protokol, SIGNAL(sessionLoadFailed()));
    QSignalSpy estabSpy(&protokol, SIGNAL(sessionEstablished(QString,bool)));
    QSignalSpy promptSpy(&protokol, SIGNAL(promptResponded(QString)));
    QSignalSpy chunkSpy(&protokol, SIGNAL(agentMessageChunk(QString)));
    QSignalSpy toolSpy(&protokol, SIGNAL(toolCallAdded(QString,QString,QString)));
    QSignalSpy diffSpy(&protokol, SIGNAL(diffContent(QString,QString)));
    QSignalSpy permSpy(&protokol, SIGNAL(permissionRequestReceived(qint64,QJsonObject)));
    QSignalSpy outSpy(&protokol, SIGNAL(messageOut(AcpMessage)));
    QSignalSpy endSpy(&protokol, SIGNAL(roundEnded()));

    qint64 initId = protokol.registerRequest("initialize");
    QJsonObject initResult;
    initResult.insert("protocolVersion", 1);
    protokol.handleMessage(AcpMessage::response(initId, initResult));
    QCOMPARE(initSpy.count(), 1);
    QCOMPARE(initSpy.at(0).at(0).toJsonObject().value("protocolVersion").toInt(), 1);

    qint64 errId = protokol.registerRequest("initialize");
    protokol.handleMessage(AcpMessage::errorResponse(errId, -32600, "ugyldig anmodning"));
    QCOMPARE(errSpy.count(), 1);
    QCOMPARE(errSpy.at(0).at(0).toString(), QString("initialize"));
    QCOMPARE(errSpy.at(0).at(1).toString(), QString("ugyldig anmodning"));

    qint64 loadId = protokol.registerRequest("session/load");
    protokol.handleMessage(AcpMessage::errorResponse(loadId, -32602, "ingen session"));
    QCOMPARE(loadFailSpy.count(), 1);

    qint64 newId = protokol.registerRequest("session/new");
    QJsonObject newResult;
    newResult.insert("sessionId", "abc");
    protokol.handleMessage(AcpMessage::response(newId, newResult));
    QCOMPARE(estabSpy.count(), 1);
    QCOMPARE(estabSpy.at(0).at(0).toString(), QString("abc"));
    QCOMPARE(estabSpy.at(0).at(1).toBool(), false);

    qint64 promptId = protokol.registerRequest("session/prompt");
    QJsonObject promptResult;
    promptResult.insert("stopReason", "end_turn");
    protokol.handleMessage(AcpMessage::response(promptId, promptResult));
    QCOMPARE(promptSpy.count(), 1);
    QCOMPARE(promptSpy.at(0).at(0).toString(), QString("end_turn"));

    QJsonObject chunkUpdate;
    chunkUpdate.insert("sessionUpdate", "agent_message_chunk");
    QJsonObject chunkContent;
    chunkContent.insert("type", "text");
    chunkContent.insert("text", "Hej");
    chunkUpdate.insert("content", chunkContent);
    QJsonObject chunkParams;
    chunkParams.insert("update", chunkUpdate);
    protokol.handleMessage(AcpMessage::notification("session/update", chunkParams));
    QCOMPARE(chunkSpy.count(), 1);
    QCOMPARE(chunkSpy.at(0).at(0).toString(), QString("Hej"));

    QJsonObject toolUpdate;
    toolUpdate.insert("sessionUpdate", "tool_call");
    toolUpdate.insert("toolCallId", "tc1");
    toolUpdate.insert("title", QString::fromUtf8("Læs fil"));
    toolUpdate.insert("status", "pending");
    QJsonObject diffBlock;
    diffBlock.insert("type", "diff");
    diffBlock.insert("path", "/tmp/a.txt");
    diffBlock.insert("diff", "@@ -1 +1 @@");
    QJsonArray blocks;
    blocks.append(diffBlock);
    toolUpdate.insert("content", blocks);
    QJsonObject toolParams;
    toolParams.insert("update", toolUpdate);
    protokol.handleMessage(AcpMessage::notification("session/update", toolParams));
    QCOMPARE(toolSpy.count(), 1);
    QCOMPARE(toolSpy.at(0).at(0).toString(), QString("tc1"));
    QCOMPARE(diffSpy.count(), 1);
    QCOMPARE(diffSpy.at(0).at(0).toString(), QString("/tmp/a.txt"));

    QJsonObject permParams;
    permParams.insert("reason", "vil skrive");
    protokol.handleMessage(AcpMessage::request(42, "session/request_permission", permParams));
    QCOMPARE(permSpy.count(), 1);
    QCOMPARE(permSpy.at(0).at(0).toLongLong(), qint64(42));

    protokol.handleMessage(AcpMessage::request(43, "frobnicate", QJsonObject()));
    QCOMPARE(outSpy.count(), 1);
    AcpMessage svar = outSpy.at(0).at(0).value<AcpMessage>();
    QCOMPARE(svar.type(), AcpMessage::Response);
    QCOMPARE(svar.id(), qint64(43));
    QVERIFY(svar.isError());
    QCOMPARE(svar.error().value("code").toInt(), -32601);

    protokol.handleMessage(AcpMessage::notification("end_turn", QJsonObject()));
    QCOMPARE(endSpy.count(), 1);
}

void TestArkitekturOpdelingProtokol::request_id_bogfoering_i_acp_protokol()
{
    AcpProtokol protokol;
    QCOMPARE(protokol.pendingRequestCount(), 0);
    qint64 a = protokol.registerRequest("initialize");
    qint64 b = protokol.registerRequest("session/new");
    QVERIFY(b > a);
    QCOMPARE(protokol.pendingRequestCount(), 2);
    QVERIFY(protokol.hasPendingRequest(a));
    QCOMPARE(protokol.takeRequestMethod(a), QString("initialize"));
    QCOMPARE(protokol.pendingRequestCount(), 1);
    QVERIFY(!protokol.hasPendingRequest(a));
    QCOMPARE(protokol.takeRequestMethod(999), QString());
    protokol.clearPendingRequests();
    QCOMPARE(protokol.pendingRequestCount(), 0);
    QVERIFY(!protokol.hasPendingRequest(b));
}

void TestArkitekturOpdelingProtokol::acpclient_har_ingen_dispatch()
{
    QString header = readSource("src/acpclient.h");
    QString kilde = readSource("src/acpclient.cpp");
    QVERIFY(!header.isEmpty());
    QVERIFY(!kilde.isEmpty());
    QVERIFY(!header.contains("m_pendingRequests"));
    QVERIFY(!header.contains("handleMessage"));
    QVERIFY(!header.contains("handleSessionUpdate"));
    QVERIFY(!kilde.contains("AcpClient::handleMessage"));
    QVERIFY(!kilde.contains("AcpClient::handleSessionUpdate"));
    QVERIFY(!kilde.contains("AcpClient::handleNotification"));
    QVERIFY(!kilde.contains("m_pendingRequests"));

    QString protokolKilde = readSource("src/acpprotokol.cpp");
    QString sessionKilde = readSource("src/acpsession.cpp");
    QCOMPARE(kilde.count("session/prompt"), 0);
    QCOMPARE(kilde.count("session/cancel"), 0);
    QCOMPARE(protokolKilde.count("session/prompt"), 1);
    QCOMPARE(sessionKilde.count("session/cancel"), 1);
    QCOMPARE(kilde.count("set_config_option"), 1);

    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy initSpy(&client, SIGNAL(initializeResponse(QJsonObject)));
    QSignalSpy readySpy(&client, SIGNAL(agentReady()));
    client.sendInitialize();
    QByteArray svar = "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"protocolVersion\":1,\"configOptions\":[]}}";
    client.feedBytesForTest(svar + "\n");
    QCOMPARE(initSpy.count(), 1);
    QCOMPARE(readySpy.count(), 1);
}

void TestArkitekturOpdelingProtokol::eksisterende_rygrad_tests_stadig_groenne()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy droppet(&reader, SIGNAL(lineDropped(QByteArray,QString)));
    reader.feedBytesForTest(readFixture("fixture_rygrad.ndjson"));
    QCOMPARE(klar.count(), 5);
    QCOMPARE(droppet.count(), 0);
    for (int i = 0; i < klar.count(); ++i) {
        QByteArray line = klar.at(i).at(0).toByteArray();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        QCOMPARE(err.error, QJsonParseError::NoError);
        QVERIFY(doc.isObject());
    }

    QStringList navne;
    for (int i = 0; i < testRegistrations().size(); ++i)
        navne.append(testRegistrations().at(i).name);
    QVERIFY(navne.contains("TestAcpRygrad"));
    QVERIFY(navne.contains("TestArkitekturOpdelingProtokol"));
}

#include "tst_arkitektur_opdeling_protokol.moc"
