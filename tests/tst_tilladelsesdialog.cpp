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
    QByteArray readFixture(const QString& name);
};

REGISTER_TEST(TestTilladelsesdialog)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestTilladelsesdialog::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + "/skrynja-tst-settings-tilladelse";
    QDir().mkpath(path);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings s;
    s.clear();
    s.sync();
}

QByteArray TestTilladelsesdialog::readFixture(const QString& name)
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

void TestTilladelsesdialog::anmodning_parses_til_pendingPermission()
{
    AcpClient client;
    client.setTestMode(true);
    QSignalSpy requested(&client, SIGNAL(permissionRequested()));
    client.feedBytesForTest(readFixture("fixture_tilladelse.ndjson"));
    QCOMPARE(requested.count(), 1);
    PermissionModel* model = client.permissionModel();
    QVERIFY(model->isPending());
    QVERIFY(!model->pendingPermission().isEmpty());
    QCOMPARE(model->requestId(), 100);
}

void TestTilladelsesdialog::options_liste_med_optionId()
{
    AcpClient client;
    client.setTestMode(true);
    client.feedBytesForTest(readFixture("fixture_tilladelse.ndjson"));
    QVariantMap pending = client.permissionModel()->pendingPermission();
    QVariantList options = pending.value("options").toList();
    QCOMPARE(options.size(), 2);
    QCOMPARE(options.at(0).toMap().value("optionId").toString(), QString("allow"));
    QCOMPARE(options.at(1).toMap().value("optionId").toString(), QString("deny"));
    QCOMPARE(pending.value("toolCall").toMap().value("title").toString(), QString("Skriv fil"));
}

void TestTilladelsesdialog::svar_payload_selected_optionId()
{
    AcpClient client;
    client.setTestMode(true);
    client.feedBytesForTest(readFixture("fixture_tilladelse.ndjson"));
    QVERIFY(client.answerPermission("allow"));
    QJsonObject o = jsonObj(client.lastSentForTest());
    QCOMPARE(o.value("id").toVariant().toLongLong(), 100);
    QJsonObject outcome = o.value("result").toObject().value("outcome").toObject();
    QCOMPARE(outcome.value("outcome").toString(), QString("selected"));
    QCOMPARE(outcome.value("optionId").toString(), QString("allow"));
}

void TestTilladelsesdialog::afvisning_giver_outcome_cancelled()
{
    AcpClient client;
    client.setTestMode(true);
    client.feedBytesForTest(readFixture("fixture_tilladelse.ndjson"));
    QVERIFY(client.declinePermission());
    QJsonObject o = jsonObj(client.lastSentForTest());
    QJsonObject outcome = o.value("result").toObject().value("outcome").toObject();
    QCOMPARE(outcome.value("outcome").toString(), QString("cancelled"));
}

void TestTilladelsesdialog::ingen_dobbelt_svar()
{
    AcpClient client;
    client.setTestMode(true);
    client.feedBytesForTest(readFixture("fixture_tilladelse.ndjson"));
    QCOMPARE(client.permissionModel()->requests(), 1);
    QVERIFY(client.answerPermission("allow"));
    QVERIFY(!client.answerPermission("deny"));
    QVERIFY(!client.declinePermission());
    int responseCount = 0;
    QList<QByteArray> sent = client.sentForTest();
    for (int i = 0; i < sent.size(); ++i) {
        QJsonObject o = jsonObj(sent.at(i));
        if (o.contains("id") && o.value("id").toVariant().toLongLong() == 100 && o.contains("result"))
            ++responseCount;
    }
    QCOMPARE(responseCount, 1);
}

#include "tst_tilladelsesdialog.moc"
