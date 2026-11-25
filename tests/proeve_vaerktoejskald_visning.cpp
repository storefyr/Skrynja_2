#include <QtTest/QtTest>
#include <QSignalSpy>
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
    void qml_bruger_en_eneste_model_mekanisme();
private:
};

REGISTER_TEST(TestVaerktoejskaldVisning)

void TestVaerktoejskaldVisning::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-vaerktoej");
}

void TestVaerktoejskaldVisning::tool_call_felter_parses()
{
    TestClient client;
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
    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_værktøjskald.ndjson"));
    ToolCallModel* model = client.toolCallModel();
    QCOMPARE(model->find("tc-1").value("status").toString(), QString("completed"));
    QCOMPARE(model->find("tc-2").value("status").toString(), QString("in_progress"));
}

void TestVaerktoejskaldVisning::ui_kvitterer_sete_kald()
{
    TestClient client;
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
    TestClient client;
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

    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.feedBytesForTest(testReadFixture("fixture_v\u00e6rkt\u00f8jskald.ndjson"));
    ToolCallModel* model = client.toolCallModel();
    QCOMPARE(model->toolCalls().size(), model->count());
    QVariantMap viaList = model->toolCalls().at(0).toMap();
    QVariantMap viaFind = model->find(viaList.value("toolCallId").toString());
    QCOMPARE(viaList, viaFind);
}

void TestVaerktoejskaldVisning::qml_bruger_en_eneste_model_mekanisme()
{
    QStringList modelFiler;
    modelFiler << "src/conversationmodel.h" << "src/conversationmodel.cpp"
               << "src/toolcallmodel.h" << "src/toolcallmodel.cpp"
               << "src/diffmodel.h" << "src/diffmodel.cpp";
    for (int i = 0; i < modelFiler.size(); ++i) {
        QString src = testReadSource(modelFiler.at(i));
        QVERIFY2(!src.isEmpty(), qPrintable("mangler " + modelFiler.at(i)));
        QVERIFY2(!src.contains("QAbstractListModel"),
                 qPrintable("QAbstractListModel stadig i " + modelFiler.at(i)));
    }
    QVERIFY(testReadSource("src/conversationmodel.h").contains("changed()"));
    QVERIFY(testReadSource("src/toolcallmodel.h").contains("QVariantList"));
    QVERIFY(testReadSource("src/diffmodel.h").contains("QVariantList"));
    QStringList cppFiler;
    cppFiler << "src/conversationmodel.cpp" << "src/toolcallmodel.cpp" << "src/diffmodel.cpp";
    for (int i = 0; i < cppFiler.size(); ++i) {
        QString src = testReadSource(cppFiler.at(i));
        QVERIFY2(!src.contains("beginInsertRows") && !src.contains("rowCount"),
                 qPrintable("model-roller stadig i " + cppFiler.at(i)));
    }

    QStringList qmlRelative;
    qmlRelative << "qml/harbour-skrynja-2.qml";
    QDir pagesDir(testRepoRoot() + "/qml/pages");
    QStringList pages = pagesDir.entryList(QStringList() << "*.qml", QDir::Files);
    for (int i = 0; i < pages.size(); ++i)
        qmlRelative << "qml/pages/" + pages.at(i);
    QVERIFY(qmlRelative.size() >= 5);
    QStringList tilladteLister;
    tilladteLister << "toolCalls" << "diffs" << "outputs" << "configOptions";
    QRegExp modelRe("model:\\s*acp\\.(\\w+)");
    for (int i = 0; i < qmlRelative.size(); ++i) {
        QString src = testReadSource(qmlRelative.at(i));
        QVERIFY2(!src.isEmpty(), qPrintable("mangler " + qmlRelative.at(i)));
        QVERIFY2(!src.contains("QAbstractListModel"),
                 qPrintable("QAbstractListModel i " + qmlRelative.at(i)));
        QVERIFY2(!src.contains("acp.conversationModel") && !src.contains("acp.toolCallModel")
                 && !src.contains("acp.diffModel"),
                 qPrintable("qml bruger model-objekt direkte i " + qmlRelative.at(i)));
        int pos = 0;
        while ((pos = modelRe.indexIn(src, pos)) != -1) {
            QVERIFY2(tilladteLister.contains(modelRe.cap(1)),
                     qPrintable("qml model: er ikke en QVariantList-property: " + modelRe.cap(1)
                                + " i " + qmlRelative.at(i)));
            pos += modelRe.matchedLength();
        }
    }

    TestClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy changedSpy(client.toolCallModel(), SIGNAL(changed()));
    client.feedBytesForTest(testReadFixture("fixture_v\u00e6rkt\u00f8jskald.ndjson"));
    QCOMPARE(client.toolCalls().size(), 2);
    QCOMPARE(client.toolCallModel()->toolCalls().size(), 2);
    QVERIFY(changedSpy.count() > 0);
}

#include "proeve_vaerktoejskald_visning.moc"
