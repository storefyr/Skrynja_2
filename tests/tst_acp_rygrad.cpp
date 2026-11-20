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
    QByteArray læsFixture(const QString& navn);
};

REGISTRER_TEST(TestAcpRygrad)

QByteArray TestAcpRygrad::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestAcpRygrad::rygrad_fixture_hoved()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    QSignalSpy droppet(&læser, SIGNAL(linjeDroppet(QByteArray,QString)));
    læser.feedBytesForTest(læsFixture("fixture_rygrad.ndjson"));
    QCOMPARE(klar.count(), 5);
    QCOMPARE(droppet.count(), 0);
    for (int i = 0; i < klar.count(); ++i) {
        QByteArray linje = klar.at(i).at(0).toByteArray();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(linje, &err);
        QCOMPARE(err.error, QJsonParseError::NoError);
        QVERIFY(doc.isObject());
    }
}

void TestAcpRygrad::rygrad_sammenklaebning()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    QSignalSpy droppet(&læser, SIGNAL(linjeDroppet(QByteArray,QString)));
    QByteArray data = læsFixture("fixture_rygrad_sammenklaebning.ndjson");
    QVERIFY(!data.isEmpty());
    for (int i = 0; i < data.size(); i += 7)
        læser.feedBytesForTest(data.mid(i, 7));
    QCOMPARE(droppet.count(), 0);
    QCOMPARE(klar.count(), 3);
    QByteArray første = klar.at(0).at(0).toByteArray();
    QVERIFY(første.contains("\"id\":10"));
    QJsonParseError err;
    QVERIFY(QJsonDocument::fromJson(første, &err).isObject());
    QCOMPARE(err.error, QJsonParseError::NoError);
}

void TestAcpRygrad::rygrad_ugyldig_ruller_tilbage()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    QSignalSpy droppet(&læser, SIGNAL(linjeDroppet(QByteArray,QString)));
    læser.feedBytesForTest(læsFixture("fixture_rygrad_ugyldig.ndjson"));
    QCOMPARE(klar.count(), 3);
    QCOMPARE(droppet.count(), 2);
    læser.feedBytesForTest("{\"jsonrpc\":\"2.0\",\"id\":3,\"result\":{}}\n");
    QCOMPARE(klar.count(), 4);
    QCOMPARE(droppet.count(), 2);
}

void TestAcpRygrad::rygrad_utf8_ugyldig()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    QSignalSpy droppet(&læser, SIGNAL(linjeDroppet(QByteArray,QString)));
    QByteArray data = læsFixture("fixture_rygrad_utf8_ugyldig.ndjson");
    QVERIFY(!data.isEmpty());
    læser.feedBytesForTest(data);
    QCOMPARE(klar.count(), 2);
    QCOMPARE(droppet.count(), 1);
    QCOMPARE(droppet.at(0).at(1).toString(), QString("ugyldig utf-8"));
}

void TestAcpRygrad::rygrad_crlf()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    læser.feedBytesForTest("{\"a\":1}\r\n{\"b\":2}\r\n");
    QCOMPARE(klar.count(), 2);
    for (int i = 0; i < klar.count(); ++i)
        QVERIFY(!klar.at(i).at(0).toByteArray().contains('\r'));
}

void TestAcpRygrad::rygrad_flere_paa_en_linje()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    læser.feedBytesForTest("{\"a\":1}{\"b\":2}\n");
    QCOMPARE(klar.count(), 2);
    QCOMPARE(QJsonDocument::fromJson(klar.at(0).at(0).toByteArray()).object().value("a").toInt(), 1);
    QCOMPARE(QJsonDocument::fromJson(klar.at(1).at(0).toByteArray()).object().value("b").toInt(), 2);
}

void TestAcpRygrad::rygrad_cap_droppes()
{
    AcpFrameReader læser(0);
    læser.sætCap(16);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    QSignalSpy droppet(&læser, SIGNAL(linjeDroppet(QByteArray,QString)));
    QByteArray lang = "{\"tekst\":\"denne linje er alt for lang til cap\"}";
    læser.feedBytesForTest(lang + "\n{\"ok\":1}\n");
    QCOMPARE(droppet.count(), 1);
    QCOMPARE(droppet.at(0).at(1).toString(), QString("cap"));
    QCOMPARE(klar.count(), 1);
}

void TestAcpRygrad::rygrad_hale_ved_eof()
{
    AcpFrameReader læser(0);
    QSignalSpy klar(&læser, SIGNAL(beskedKlar(QByteArray)));
    QSignalSpy hale(&læser, SIGNAL(haleReady(QByteArray)));
    læser.feedBytesForTest("{\"delvis\":1");
    QCOMPARE(klar.count(), 0);
    læser.eofForTest();
    QCOMPARE(hale.count(), 1);
    QCOMPARE(hale.at(0).at(0).toByteArray(), QByteArray("{\"delvis\":1"));
    QCOMPARE(klar.count(), 0);
}

void TestAcpRygrad::besked_id_korrelation()
{
    QJsonObject params;
    params.insert("protocolVersion", 1);
    AcpBesked anmodning = AcpBesked::anmodning(7, "session/prompt", params);
    bool ok = false;
    AcpBesked læst = AcpBesked::fraJson(anmodning.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(læst.type(), AcpBesked::Anmodning);
    QVERIFY(læst.harId());
    QCOMPARE(læst.id(), 7);
    QCOMPARE(læst.metode(), QString("session/prompt"));
    QCOMPARE(læst.params().value("protocolVersion").toInt(), 1);

    AcpBesked svar = AcpBesked::svar(7, QJsonValue(42));
    AcpBesked svarLæst = AcpBesked::fraJson(svar.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(svarLæst.type(), AcpBesked::Svar);
    QCOMPARE(svarLæst.id(), 7);
    QCOMPARE(svarLæst.resultat().toInt(), 42);
    QVERIFY(!svarLæst.erFejl());

    AcpBesked fejl = AcpBesked::fejlSvar(9, -32601, "ukendt metode");
    AcpBesked fejlLæst = AcpBesked::fraJson(fejl.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(fejlLæst.type(), AcpBesked::Svar);
    QVERIFY(fejlLæst.erFejl());
    QCOMPARE(fejlLæst.fejl().value("code").toInt(), -32601);

    AcpBesked notif = AcpBesked::notifikation("end_turn", QJsonObject());
    QVERIFY(!notif.harId());
    AcpBesked notifLæst = AcpBesked::fraJson(notif.tilJson(), &ok);
    QVERIFY(ok);
    QCOMPARE(notifLæst.type(), AcpBesked::Notifikation);
    QCOMPARE(notifLæst.metode(), QString("end_turn"));

    AcpBesked::fraJson("ikke json overhovedet", &ok);
    QVERIFY(!ok);
}

#include "tst_acp_rygrad.moc"
