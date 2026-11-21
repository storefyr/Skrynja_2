#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "acpclient.h"
#include "acpbesked.h"
#include "konfigurationsmodel.h"
#include "diffmodel.h"
#include "samtalemodel.h"
#include "tilladelsesmodel.h"
#include "vaerktoejskaldsmodel.h"
#include "testregistrering.h"

class TestArkitekturOpdelingUiModeller : public QObject
{
    Q_OBJECT
private slots:
    void acpclient_er_tynd_facade();
    void fem_modeller_uden_acpclient_ejerskab();
    void q_properties_uenaendret_for_qml();
    void eksisterende_model_tests_stadig_groenne();
private:
};

REGISTER_TEST(TestArkitekturOpdelingUiModeller)

void TestArkitekturOpdelingUiModeller::acpclient_er_tynd_facade()
{
    QString header = testReadSource("src/acpclient.h");
    QString kilde = testReadSource("src/acpclient.cpp");
    QVERIFY(!header.isEmpty());
    QVERIFY(!kilde.isEmpty());

    QVERIFY(!kilde.contains("new ConversationModel"));
    QVERIFY(!kilde.contains("new ToolCallModel"));
    QVERIFY(!kilde.contains("new PermissionModel"));
    QVERIFY(!kilde.contains("new DiffModel"));
    QVERIFY(!kilde.contains("new ConfigurationModel"));

    QVERIFY(header.contains("setConversationModel"));
    QVERIFY(header.contains("setToolCallModel"));
    QVERIFY(header.contains("setPermissionModel"));
    QVERIFY(header.contains("setDiffModel"));
    QVERIFY(header.contains("setConfigurationModel"));

    AcpClient client;
    const char* properties[] = {
        "hadRestoredHistory", "toolCalls", "conversationText", "conversationRound",
        "conversationStatus", "pendingPermission", "permissionCount", "diffs",
        "outputs", "configOptions", "agentPath"
    };
    for (int i = 0; i < 11; ++i)
        QVERIFY2(client.metaObject()->indexOfProperty(properties[i]) >= 0, properties[i]);
}

void TestArkitekturOpdelingUiModeller::fem_modeller_uden_acpclient_ejerskab()
{
    QObject ejer;
    ConversationModel samtale(&ejer);
    ToolCallModel vaerktoejskald(&ejer);
    PermissionModel tilladelse(&ejer);
    DiffModel diff(&ejer);
    ConfigurationModel konfiguration(&ejer);

    AcpClient client;
    client.setConversationModel(&samtale);
    client.setToolCallModel(&vaerktoejskald);
    client.setPermissionModel(&tilladelse);
    client.setDiffModel(&diff);
    client.setConfigurationModel(&konfiguration);

    QCOMPARE(client.conversationModel(), &samtale);
    QCOMPARE(client.toolCallModel(), &vaerktoejskald);
    QCOMPARE(client.permissionModel(), &tilladelse);
    QCOMPARE(client.diffModel(), &diff);
    QCOMPARE(client.configurationModel(), &konfiguration);

    QVERIFY(samtale.parent() != &client);
    QVERIFY(vaerktoejskald.parent() != &client);
    QVERIFY(tilladelse.parent() != &client);
    QVERIFY(diff.parent() != &client);
    QVERIFY(konfiguration.parent() != &client);
    QCOMPARE(samtale.parent(), &ejer);
}

void TestArkitekturOpdelingUiModeller::q_properties_uenaendret_for_qml()
{
    QString header = testReadSource("src/acpclient.h");
    QVERIFY(!header.isEmpty());
    QCOMPARE(header.count("Q_PROPERTY(bool hadRestoredHistory"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QVariantList toolCalls"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QString conversationText"), 1);
    QCOMPARE(header.count("Q_PROPERTY(int conversationRound"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QString conversationStatus"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QVariantMap pendingPermission"), 1);
    QCOMPARE(header.count("Q_PROPERTY(int permissionCount"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QVariantList diffs"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QVariantList outputs"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QVariantList configOptions"), 1);
    QCOMPARE(header.count("Q_PROPERTY(QString agentPath"), 1);

    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    QSignalSpy convSpy(&client, SIGNAL(conversationChanged()));
    QSignalSpy toolSpy(&client, SIGNAL(toolCallsChanged()));
    QSignalSpy permSpy(&client, SIGNAL(permissionRequested()));

    QByteArray chunk = "{\"jsonrpc\":\"2.0\",\"method\":\"session/update\",\"params\":{\"update\":{\"sessionUpdate\":\"agent_message_chunk\",\"content\":{\"type\":\"text\",\"text\":\"Hej\"}}}}";
    client.feedBytesForTest(chunk + "\n");
    QCOMPARE(client.conversationText(), QString("Hej"));
    QVERIFY(convSpy.count() >= 1);

    QByteArray tool = "{\"jsonrpc\":\"2.0\",\"method\":\"session/update\",\"params\":{\"update\":{\"sessionUpdate\":\"tool_call\",\"toolCallId\":\"tc1\",\"title\":\"t\",\"status\":\"pending\",\"content\":[]}}}";
    client.feedBytesForTest(tool + "\n");
    QCOMPARE(client.toolCalls().size(), 1);
    QVERIFY(toolSpy.count() >= 1);

    QByteArray perm = "{\"jsonrpc\":\"2.0\",\"id\":50,\"method\":\"session/request_permission\",\"params\":{\"options\":[]}}";
    client.feedBytesForTest(perm + "\n");
    QVERIFY(permSpy.count() >= 1);
    QVERIFY(!client.pendingPermission().isEmpty());
}

void TestArkitekturOpdelingUiModeller::eksisterende_model_tests_stadig_groenne()
{
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);

    client.sendInitialize();
    qint64 initId = testIdOf(client.lastSentForTest());
    QJsonObject option;
    option.insert("configId", "model");
    option.insert("name", "Model");
    QJsonArray options;
    options.append(option);
    QJsonObject initResult;
    initResult.insert("protocolVersion", 1);
    initResult.insert("configOptions", options);
    client.feedBytesForTest(testResponseJson(initId, initResult) + "\n");
    QCOMPARE(client.configOptions().size(), 1);
    QCOMPARE(client.configurationModel()->configOptions().size(), 1);

    QStringList navne;
    for (int i = 0; i < testRegistrations().size(); ++i)
        navne.append(testRegistrations().at(i).name);
    QVERIFY(navne.contains("TestPromptOgSvar"));
    QVERIFY(navne.contains("TestVaerktoejskaldVisning"));
    QVERIFY(navne.contains("TestTilladelsesdialog"));
    QVERIFY(navne.contains("TestDiffOgUddata"));
    QVERIFY(navne.contains("TestKonfigurationAfAgent"));
    QVERIFY(navne.contains("TestArkitekturOpdelingUiModeller"));
}

#include "tst_arkitektur_opdeling_ui_modeller.moc"
