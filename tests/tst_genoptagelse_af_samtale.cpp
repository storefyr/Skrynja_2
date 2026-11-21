#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include "acpclient.h"
#include "sessionstore.h"
#include "testregistrering.h"

class TestGenoptagelseAfSamtale : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void session_id_gemmes_i_settings();
    void load_med_gemt_session_id();
    void fallback_til_new_ved_load_fejl();
    void hadRestoredHistory_saettes_korrekt();
    void resumeregel_laeser_manifest_og_finder_foerste_false();
private:
};

REGISTER_TEST(TestGenoptagelseAfSamtale)

void TestGenoptagelseAfSamtale::initTestCase()
{
    opretTestSettingsSti("/skrynja-tst-settings-genoptag");
}

void TestGenoptagelseAfSamtale::session_id_gemmes_i_settings()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionStore store;
    QVERIFY(!store.hasSessionId());
    store.saveSessionId("sess-abc");
    QVERIFY(store.hasSessionId());
    QCOMPARE(store.sessionId(), QString("sess-abc"));
    SessionStore igen;
    QCOMPARE(igen.sessionId(), QString("sess-abc"));
}

void TestGenoptagelseAfSamtale::load_med_gemt_session_id()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionStore store;
    store.saveSessionId("sess-gammel");
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.startSession();
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/load"));
    QCOMPARE(o.value("params").toObject().value("sessionId").toString(), QString("sess-gammel"));
}

void TestGenoptagelseAfSamtale::fallback_til_new_ved_load_fejl()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionStore store;
    store.saveSessionId("sess-gammel");
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.startSession();
    qint64 id = testIdOf(client.lastSentForTest());
    client.feedBytesForTest(testErrorResponseJson(id, -32602, "ukendt session") + "\n");
    QJsonObject o = testJsonObj(client.lastSentForTest());
    QCOMPARE(o.value("method").toString(), QString("session/new"));
    QVERIFY(!client.hadRestoredHistory());
}

void TestGenoptagelseAfSamtale::hadRestoredHistory_saettes_korrekt()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionStore store;
    store.saveSessionId("sess-gammel");
    AcpClient client;
    injectTestModels(&client, this);
    client.setTestMode(true);
    client.startSession();
    qint64 id = testIdOf(client.lastSentForTest());
    QVERIFY(!client.hadRestoredHistory());
    QJsonObject result;
    result.insert("sessionId", "sess-gammel");
    client.feedBytesForTest(testResponseJson(id, result) + "\n");
    QVERIFY(client.hadRestoredHistory());
    QCOMPARE(client.sessionId(), QString("sess-gammel"));
}

void TestGenoptagelseAfSamtale::resumeregel_laeser_manifest_og_finder_foerste_false()
{
    QString path = QCoreApplication::applicationDirPath()
                  + "/../realiseringsmanifest_for_funktioner_og_parrede_tests.json";
    if (!QFileInfo::exists(path))
        path = "../realiseringsmanifest_for_funktioner_og_parrede_tests.json";
    QVERIFY2(QFileInfo::exists(path), qPrintable(path));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QJsonObject manifest = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    QJsonArray order = manifest.value("strategisk_rækkefølge").toArray();
    QJsonArray funktioner = manifest.value("funktioner").toArray();
    QCOMPARE(funktioner.size(), order.size());
    QHash<QString, bool> passerer;
    for (int i = 0; i < funktioner.size(); ++i)
        passerer.insert(funktioner.at(i).toObject().value("id").toString(),
                        funktioner.at(i).toObject().value("passerer").toBool());
    QString firstInOrder;
    for (int i = 0; i < order.size(); ++i) {
        QString id = order.at(i).toString();
        if (!passerer.value(id)) {
            firstInOrder = id;
            break;
        }
    }
    QString firstInFunctions;
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        if (!fn.value("passerer").toBool()) {
            firstInFunctions = fn.value("id").toString();
            break;
        }
    }
    QCOMPARE(firstInOrder, firstInFunctions);
}

#include "tst_genoptagelse_af_samtale.moc"
