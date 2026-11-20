#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QVariantMap>

#include "acpclient.h"
#include "testregistrering.h"
#include "vaerktoejskaldsmodel.h"

class TestVaerktoejskaldVisning : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void tool_call_felter_parses();
    void tool_call_update_opdaterer_status();
    void ui_kvitterer_sete_kald();
    void flere_kald_i_raekkefoelge();
private:
    QByteArray læsFixture(const QString& navn);
};

REGISTRER_TEST(TestVaerktoejskaldVisning)

void TestVaerktoejskaldVisning::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-vaerktoej";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestVaerktoejskaldVisning::læsFixture(const QString& navn)
{
    QString sti = QFINDTESTDATA("fixtures/" + navn);
    if (sti.isEmpty())
        sti = "fixtures/" + navn;
    QFile f(sti);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestVaerktoejskaldVisning::tool_call_felter_parses()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_værktøjskald.ndjson"));
    VaerktoejskaldsModel* model = klient.vaerktoejskaldsModel();
    QCOMPARE(model->count(), 2);
    QVariantMap tc1 = model->find("tc-1");
    QCOMPARE(tc1.value("toolCallId").toString(), QString("tc-1"));
    QCOMPARE(tc1.value("titel").toString(), QString("Læs fil"));
    QVERIFY(tc1.contains("status"));
}

void TestVaerktoejskaldVisning::tool_call_update_opdaterer_status()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_værktøjskald.ndjson"));
    VaerktoejskaldsModel* model = klient.vaerktoejskaldsModel();
    QCOMPARE(model->find("tc-1").value("status").toString(), QString("completed"));
    QCOMPARE(model->find("tc-2").value("status").toString(), QString("in_progress"));
}

void TestVaerktoejskaldVisning::ui_kvitterer_sete_kald()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_værktøjskald.ndjson"));
    VaerktoejskaldsModel* model = klient.vaerktoejskaldsModel();
    QVERIFY(model->acknowledge("tc-1"));
    QVERIFY(model->find("tc-1").value("set").toBool());
    QVERIFY(!model->acknowledge("tc-1"));
    QVERIFY(!model->acknowledge("ukendt-id"));
}

void TestVaerktoejskaldVisning::flere_kald_i_raekkefoelge()
{
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.feedBytesForTest(læsFixture("fixture_værktøjskald.ndjson"));
    QVariantList kald = klient.vaerktoejskaldsModel()->toolCalls();
    QCOMPARE(kald.size(), 2);
    QCOMPARE(kald.at(0).toMap().value("toolCallId").toString(), QString("tc-1"));
    QCOMPARE(kald.at(1).toMap().value("toolCallId").toString(), QString("tc-2"));
}

#include "tst_vaerktoejskald_visning.moc"
