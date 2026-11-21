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
    QByteArray readFixture(const QString& name);
};

REGISTER_TEST(TestDiffOgUddata)

void TestDiffOgUddata::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + "/skrynja-tst-settings-diff";
    QDir().mkpath(path);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestDiffOgUddata::readFixture(const QString& name)
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestDiffOgUddata::diff_blok_parses()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(readFixture("fixture_diff.ndjson"));
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
    client.feedBytesForTest(readFixture("fixture_uddata.ndjson"));
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
    QByteArray all = readFixture("fixture_diff.ndjson");
    QByteArray firstLine = all.left(all.indexOf('\n') + 1);
    client.feedBytesForTest(firstLine);
    QVERIFY(changed.count() >= 1);
    QCOMPARE(client.diffModel()->diffCount(), 1);
    QCOMPARE(client.conversationModel()->status(), QString("klar"));
}

#include "tst_diff_og_uddata.moc"
