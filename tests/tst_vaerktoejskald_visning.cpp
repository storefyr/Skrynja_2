#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QVariantMap>

#include "acpclient.h"
#include "testregistrering.h"
#include "toolcallmodel.h"

class TestVaerktoejskaldVisning : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void tool_call_felter_parses();
    void tool_call_update_opdaterer_status();
    void ui_kvitterer_sete_kald();
    void flere_kald_i_raekkefoelge();
    void enkelt_projektion_i_toolkaldsmodel();
private:
};

REGISTER_TEST(TestVaerktoejskaldVisning)

void TestVaerktoejskaldVisning::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-vaerktoej");
}

void TestVaerktoejskaldVisning::tool_call_felter_parses()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_værktøjskald.ndjson"));
    ToolCallModel* model = client.toolCallModel();
    QCOMPARE(model->count(), 2);
    QVariantMap tc1 = model->find("tc-1");
    QCOMPARE(tc1.value("toolCallId").toString(), QString("tc-1"));
    QCOMPARE(tc1.value("title").toString(), QString("Læs fil"));
    QVERIFY(tc1.contains("status"));
}

void TestVaerktoejskaldVisning::tool_call_update_opdaterer_status()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_værktøjskald.ndjson"));
    ToolCallModel* model = client.toolCallModel();
    QCOMPARE(model->find("tc-1").value("status").toString(), QString("completed"));
    QCOMPARE(model->find("tc-2").value("status").toString(), QString("in_progress"));
}

void TestVaerktoejskaldVisning::ui_kvitterer_sete_kald()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_værktøjskald.ndjson"));
    ToolCallModel* model = client.toolCallModel();
    QVERIFY(model->acknowledge("tc-1"));
    QVERIFY(model->find("tc-1").value("set").toBool());
    QVERIFY(!model->acknowledge("tc-1"));
    QVERIFY(!model->acknowledge("ukendt-id"));
}

void TestVaerktoejskaldVisning::flere_kald_i_raekkefoelge()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_værktøjskald.ndjson"));
    QVariantList call = client.toolCallModel()->toolCalls();
    QCOMPARE(call.size(), 2);
    QCOMPARE(call.at(0).toMap().value("toolCallId").toString(), QString("tc-1"));
    QCOMPARE(call.at(1).toMap().value("toolCallId").toString(), QString("tc-2"));
}

void TestVaerktoejskaldVisning::enkelt_projektion_i_toolkaldsmodel()
{
    QString kilde = testReadSource("src/toolcallmodel.cpp");
    QVERIFY(!kilde.isEmpty());
    QVERIFY(kilde.contains("projektion"));
    QCOMPARE(kilde.count("m.insert(\"toolCallId\""), 1);

    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_v\u00e6rkt\u00f8jskald.ndjson"));
    ToolCallModel* model = client.toolCallModel();
    QCOMPARE(model->toolCalls().size(), model->count());
    QVariantMap viaList = model->toolCalls().at(0).toMap();
    QVariantMap viaFind = model->find(viaList.value("toolCallId").toString());
    QCOMPARE(viaList, viaFind);
}

#include "tst_vaerktoejskald_visning.moc"
