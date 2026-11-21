#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "acpbesked.h"
#include "acpframereader.h"
#include "testregistrering.h"

class TestAcpRygrad : public QObject
{
    Q_OBJECT
private slots:
    void rygrad_fixture_hoved();
    void rygrad_sammenklaebning();
    void rygrad_ugyldig_ruller_tilbage();
    void rygrad_utf8_ugyldig();
    void rygrad_crlf();
    void rygrad_flere_paa_en_linje();
    void rygrad_cap_droppes();
    void rygrad_hale_ved_eof();
    void besked_id_korrelation();
private:
};

REGISTER_TEST(TestAcpRygrad)

void TestAcpRygrad::rygrad_fixture_hoved()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy droppet(&reader, SIGNAL(lineDropped(QByteArray,QString)));
    reader.feedBytesForTest(testReadFixture("fixture_rygrad.ndjson"));
    QCOMPARE(klar.count(), 5);
    QCOMPARE(droppet.count(), 0);
    for (int i = 0; i < klar.count(); ++i) {
        QByteArray line = klar.at(i).at(0).toByteArray();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        QCOMPARE(err.error, QJsonParseError::NoError);
        QVERIFY(doc.isObject());
    }
}

void TestAcpRygrad::rygrad_sammenklaebning()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy droppet(&reader, SIGNAL(lineDropped(QByteArray,QString)));
    QByteArray data = testReadFixture("fixture_rygrad_sammenklæbning.ndjson");
    QVERIFY(!data.isEmpty());
    for (int i = 0; i < data.size(); i += 7)
        reader.feedBytesForTest(data.mid(i, 7));
    QCOMPARE(droppet.count(), 0);
    QCOMPARE(klar.count(), 3);
    QByteArray first = klar.at(0).at(0).toByteArray();
    QVERIFY(first.contains("\"id\":10"));
    QJsonParseError err;
    QVERIFY(QJsonDocument::fromJson(first, &err).isObject());
    QCOMPARE(err.error, QJsonParseError::NoError);
}

void TestAcpRygrad::rygrad_ugyldig_ruller_tilbage()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy droppet(&reader, SIGNAL(lineDropped(QByteArray,QString)));
    reader.feedBytesForTest(testReadFixture("fixture_rygrad_ugyldig.ndjson"));
    QCOMPARE(klar.count(), 3);
    QCOMPARE(droppet.count(), 2);
    reader.feedBytesForTest("{\"jsonrpc\":\"2.0\",\"id\":3,\"result\":{}}\n");
    QCOMPARE(klar.count(), 4);
    QCOMPARE(droppet.count(), 2);
}

void TestAcpRygrad::rygrad_utf8_ugyldig()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy droppet(&reader, SIGNAL(lineDropped(QByteArray,QString)));
    QByteArray data = testReadFixture("fixture_rygrad_utf8_ugyldig.ndjson");
    QVERIFY(!data.isEmpty());
    reader.feedBytesForTest(data);
    QCOMPARE(klar.count(), 2);
    QCOMPARE(droppet.count(), 1);
    QCOMPARE(droppet.at(0).at(1).toString(), QString("ugyldig utf-8"));
}

void TestAcpRygrad::rygrad_crlf()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    reader.feedBytesForTest("{\"a\":1}\r\n{\"b\":2}\r\n");
    QCOMPARE(klar.count(), 2);
    for (int i = 0; i < klar.count(); ++i)
        QVERIFY(!klar.at(i).at(0).toByteArray().contains('\r'));
}

void TestAcpRygrad::rygrad_flere_paa_en_linje()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    reader.feedBytesForTest("{\"a\":1}{\"b\":2}\n");
    QCOMPARE(klar.count(), 2);
    QCOMPARE(QJsonDocument::fromJson(klar.at(0).at(0).toByteArray()).object().value("a").toInt(), 1);
    QCOMPARE(QJsonDocument::fromJson(klar.at(1).at(0).toByteArray()).object().value("b").toInt(), 2);
}

void TestAcpRygrad::rygrad_cap_droppes()
{
    AcpFrameReader reader(0);
    reader.setCap(16);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy droppet(&reader, SIGNAL(lineDropped(QByteArray,QString)));
    QByteArray longLine = "{\"tekst\":\"denne linje er alt for lang til cap\"}";
    reader.feedBytesForTest(longLine + "\n{\"ok\":1}\n");
    QCOMPARE(droppet.count(), 1);
    QCOMPARE(droppet.at(0).at(1).toString(), QString("cap"));
    QCOMPARE(klar.count(), 1);
}

void TestAcpRygrad::rygrad_hale_ved_eof()
{
    AcpFrameReader reader(0);
    QSignalSpy klar(&reader, SIGNAL(messageReady(QByteArray)));
    QSignalSpy hale(&reader, SIGNAL(haleReady(QByteArray)));
    reader.feedBytesForTest("{\"delvis\":1");
    QCOMPARE(klar.count(), 0);
    reader.eofForTest();
    QCOMPARE(hale.count(), 1);
    QCOMPARE(hale.at(0).at(0).toByteArray(), QByteArray("{\"delvis\":1"));
    QCOMPARE(klar.count(), 0);
}

void TestAcpRygrad::besked_id_korrelation()
{
    QJsonObject params;
    params.insert("protocolVersion", 1);
    AcpMessage request = AcpMessage::request(7, "session/prompt", params);
    bool ok = false;
    AcpMessage read = AcpMessage::fraJson(request.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(read.type(), AcpMessage::Request);
    QVERIFY(read.hasId());
    QCOMPARE(read.id(), 7);
    QCOMPARE(read.method(), QString("session/prompt"));
    QCOMPARE(read.params().value("protocolVersion").toInt(), 1);

    AcpMessage response = AcpMessage::response(7, QJsonValue(42));
    AcpMessage responseRead = AcpMessage::fraJson(response.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(responseRead.type(), AcpMessage::Response);
    QCOMPARE(responseRead.id(), 7);
    QCOMPARE(responseRead.result().toInt(), 42);
    QVERIFY(!responseRead.isError());

    AcpMessage error = AcpMessage::errorResponse(9, -32601, "ukendt metode");
    AcpMessage errorRead = AcpMessage::fraJson(error.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(errorRead.type(), AcpMessage::Response);
    QVERIFY(errorRead.isError());
    QCOMPARE(errorRead.error().value("code").toInt(), -32601);

    AcpMessage notif = AcpMessage::notification("end_turn", QJsonObject());
    QVERIFY(!notif.hasId());
    AcpMessage notificationRead = AcpMessage::fraJson(notif.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(notificationRead.type(), AcpMessage::Notification);
    QCOMPARE(notificationRead.method(), QString("end_turn"));

    AcpMessage::fraJson("ikke json overhovedet", &ok);
    QVERIFY(!ok);
}

#include "tst_acp_rygrad.moc"
