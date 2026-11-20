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
#include "acpframereader.h"
#include "diffmodel.h"
#include "samtalemodel.h"
#include "testregistrering.h"

class TestDiffOgUddata : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void diff_blok_parses();
    void diff_linjer_cappes();
    void outputs_akkumuleres();
    void diff_vises_mens_det_sker();
private:
    QByteArray læsFixture(const QString& navn);
};

REGISTRER_TEST(TestDiffOgUddata)

void TestDiffOgUddata::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-diff";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestDiffOgUddata::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestDiffOgUddata::diff_blok_parses()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_diff.ndjson"));
    DiffModel* model = klient.diffModel();
    QCOMPARE(model->diffCount(), 2);
    QVariantList diffs = model->diffs();
    QCOMPARE(diffs.at(0).toMap().value("sti").toString(), QString("/tmp/fil.txt"));
    QVERIFY(diffs.at(0).toMap().value("linjer").toStringList().contains("+ny"));
    QCOMPARE(diffs.at(1).toMap().value("sti").toString(), QString("/tmp/fil2.txt"));
}

void TestDiffOgUddata::diff_linjer_cappes()
{
    DiffModel model;
    QString langLinje(AcpFrameReader::standardCap() + 100, 'x');
    model.addDiff("/tmp/lang.txt", langLinje + "\n" + "kort");
    QCOMPARE(model.diffCount(), 1);
    QStringList linjer = model.diffs().at(0).toMap().value("linjer").toStringList();
    QCOMPARE(linjer.size(), 2);
    QVERIFY(linjer.at(0).size() <= AcpFrameReader::standardCap());
    QCOMPARE(linjer.at(0).size(), AcpFrameReader::standardCap());
    QCOMPARE(linjer.at(1), QString("kort"));
}

void TestDiffOgUddata::outputs_akkumuleres()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_uddata.ndjson"));
    DiffModel* model = klient.diffModel();
    QCOMPARE(model->outputCount(), 2);
    QVERIFY(model->outputs().contains("kommando kørte fint"));
    QVERIFY(model->outputs().contains("mere output"));
}

void TestDiffOgUddata::diff_vises_mens_det_sker()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    QSignalSpy ændret(klient.diffModel(), SIGNAL(changed()));
    QByteArray alle = læsFixture("fixture_diff.ndjson");
    QByteArray førsteLinje = alle.left(alle.indexOf('\n') + 1);
    klient.feedBytesForTest(førsteLinje);
    QVERIFY(ændret.count() >= 1);
    QCOMPARE(klient.diffModel()->diffCount(), 1);
    QCOMPARE(klient.samtaleModel()->status(), QString("klar"));
}

#include "tst_diff_og_uddata.moc"
