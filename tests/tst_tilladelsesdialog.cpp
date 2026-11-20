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
#include "tilladelsesmodel.h"

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
private:
    QByteArray læsFixture(const QString& navn);
};

REGISTRER_TEST(TestTilladelsesdialog)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestTilladelsesdialog::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-tilladelse";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestTilladelsesdialog::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestTilladelsesdialog::anmodning_parses_til_pendingPermission()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    QSignalSpy anmodet(&klient, SIGNAL(permissionAnmodet()));
    klient.feedBytesForTest(læsFixture("fixture_tilladelse.ndjson"));
    QCOMPARE(anmodet.count(), 1);
    Tilladelsesmodel* model = klient.tilladelsesmodel();
    QVERIFY(model->afventende());
    QVERIFY(!model->pendingPermission().isEmpty());
    QCOMPARE(model->anmodningsId(), 100);
}

void TestTilladelsesdialog::options_liste_med_optionId()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse.ndjson"));
    QVariantMap pending = klient.tilladelsesmodel()->pendingPermission();
    QVariantList options = pending.value("options").toList();
    QCOMPARE(options.size(), 2);
    QCOMPARE(options.at(0).toMap().value("optionId").toString(), QString("allow"));
    QCOMPARE(options.at(1).toMap().value("optionId").toString(), QString("deny"));
    QCOMPARE(pending.value("toolCall").toMap().value("title").toString(), QString("Skriv fil"));
}

void TestTilladelsesdialog::svar_payload_selected_optionId()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse.ndjson"));
    QVERIFY(klient.answerPermission("allow"));
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QCOMPARE(o.value("id").toVariant().toLongLong(), 100);
    QJsonObject outcome = o.value("result").toObject().value("outcome").toObject();
    QCOMPARE(outcome.value("outcome").toString(), QString("selected"));
    QCOMPARE(outcome.value("optionId").toString(), QString("allow"));
}

void TestTilladelsesdialog::afvisning_giver_outcome_cancelled()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse.ndjson"));
    QVERIFY(klient.afvisPermission());
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QJsonObject outcome = o.value("result").toObject().value("outcome").toObject();
    QCOMPARE(outcome.value("outcome").toString(), QString("cancelled"));
}

void TestTilladelsesdialog::ingen_dobbelt_svar()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_tilladelse.ndjson"));
    QCOMPARE(klient.tilladelsesmodel()->anmodninger(), 1);
    QVERIFY(klient.answerPermission("allow"));
    QVERIFY(!klient.answerPermission("deny"));
    QVERIFY(!klient.afvisPermission());
    int svarTalt = 0;
    QList<QByteArray> sendte = klient.sendteForTest();
    for (int i = 0; i < sendte.size(); ++i) {
        QJsonObject o = jsonObj(sendte.at(i));
        if (o.contains("id") && o.value("id").toVariant().toLongLong() == 100 && o.contains("result"))
            ++svarTalt;
    }
    QCOMPARE(svarTalt, 1);
}

#include "tst_tilladelsesdialog.moc"
