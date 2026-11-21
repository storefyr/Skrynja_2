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
#include "conversationmodel.h"
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
    void diff_test_paastaaer_kun_diff();
private:
};

REGISTER_TEST(TestDiffOgUddata)

void TestDiffOgUddata::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-diff");
}

void TestDiffOgUddata::diff_blok_parses()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_diff.ndjson"));
    DiffModel* model = client.diffModel();
    QCOMPARE(model->diffCount(), 2);
    QVariantList diffs = model->diffs();
    QCOMPARE(diffs.at(0).toMap().value("path").toString(), QString("/tmp/fil.txt"));
    QVERIFY(diffs.at(0).toMap().value("lines").toStringList().contains("+ny"));
    QCOMPARE(diffs.at(1).toMap().value("path").toString(), QString("/tmp/fil2.txt"));
}

void TestDiffOgUddata::diff_linjer_cappes()
{
    DiffModel model;
    QString longLine(AcpFrameReader::standardCap() + 100, 'x');
    model.addDiff("/tmp/lang.txt", longLine + "\n" + "kort");
    QCOMPARE(model.diffCount(), 1);
    QStringList lines = model.diffs().at(0).toMap().value("lines").toStringList();
    QCOMPARE(lines.size(), 2);
    QVERIFY(lines.at(0).size() <= AcpFrameReader::standardCap());
    QCOMPARE(lines.at(0).size(), AcpFrameReader::standardCap());
    QCOMPARE(lines.at(1), QString("kort"));
}

void TestDiffOgUddata::outputs_akkumuleres()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_uddata.ndjson"));
    DiffModel* model = client.diffModel();
    QCOMPARE(model->outputCount(), 2);
    QVERIFY(model->outputs().contains("kommando kørte fint"));
    QVERIFY(model->outputs().contains("mere output"));
}

void TestDiffOgUddata::diff_vises_mens_det_sker()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy changed(client.diffModel(), SIGNAL(changed()));
    QByteArray all = testReadFixture("fixture_diff.ndjson");
    QByteArray firstLine = all.left(all.indexOf('\n') + 1);
    client.feedBytesForTest(firstLine);
    QVERIFY(changed.count() >= 1);
    QCOMPARE(client.diffModel()->diffCount(), 1);
}

void TestDiffOgUddata::diff_test_paastaaer_kun_diff()
{
    QString kilde = testReadSource("tests/tst_diff_og_uddata.cpp");
    int pos = kilde.indexOf("TestDiffOgUddata::diff_vises_mens_det_sker");
    QVERIFY(pos >= 0);
    QString kroppen = kilde.mid(pos, kilde.indexOf("\n}\n", pos));
    QVERIFY2(!kroppen.contains("conversationStatus"), "diff-test påstår stadig conversationStatus");
}

#include "tst_diff_og_uddata.moc"
