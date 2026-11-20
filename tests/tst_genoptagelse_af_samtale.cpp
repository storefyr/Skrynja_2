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
#include "sessionlager.h"
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
    qint64 idAf(const QByteArray& json);
    QByteArray fejlSvarJson(qint64 id, int kode, const QString& besked);
    QByteArray svarJson(qint64 id, const QJsonObject& resultat);
};

REGISTRER_TEST(TestGenoptagelseAfSamtale)

static QJsonObject jsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

void TestGenoptagelseAfSamtale::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-settings-genoptag";
    QDir().mkpath(sti);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, sti);
    QSettings s;
    s.clear();
    s.sync();
}

qint64 TestGenoptagelseAfSamtale::idAf(const QByteArray& json)
{
    return jsonObj(json).value("id").toVariant().toLongLong();
}

QByteArray TestGenoptagelseAfSamtale::fejlSvarJson(qint64 id, int kode, const QString& besked)
{
    QJsonObject fejl;
    fejl.insert("code", kode);
    fejl.insert("message", besked);
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("error", fejl);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

QByteArray TestGenoptagelseAfSamtale::svarJson(qint64 id, const QJsonObject& resultat)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", resultat);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

void TestGenoptagelseAfSamtale::session_id_gemmes_i_settings()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionLager lager;
    QVERIFY(!lager.harSessionId());
    lager.gemSessionId("sess-abc");
    QVERIFY(lager.harSessionId());
    QCOMPARE(lager.sessionId(), QString("sess-abc"));
    SessionLager igen;
    QCOMPARE(igen.sessionId(), QString("sess-abc"));
}

void TestGenoptagelseAfSamtale::load_med_gemt_session_id()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionLager lager;
    lager.gemSessionId("sess-gammel");
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.startSession();
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QCOMPARE(o.value("method").toString(), QString("session/load"));
    QCOMPARE(o.value("params").toObject().value("sessionId").toString(), QString("sess-gammel"));
}

void TestGenoptagelseAfSamtale::fallback_til_new_ved_load_fejl()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionLager lager;
    lager.gemSessionId("sess-gammel");
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.startSession();
    qint64 id = idAf(klient.sidsteSendtForTest());
    klient.feedBytesForTest(fejlSvarJson(id, -32602, "ukendt session") + "\n");
    QJsonObject o = jsonObj(klient.sidsteSendtForTest());
    QCOMPARE(o.value("method").toString(), QString("session/new"));
    QVERIFY(!klient.hadRestoredHistory());
}

void TestGenoptagelseAfSamtale::hadRestoredHistory_saettes_korrekt()
{
    QSettings s;
    s.clear();
    s.sync();
    SessionLager lager;
    lager.gemSessionId("sess-gammel");
    AcpClient klient;
    klient.sætTestTilstand(true);
    klient.startSession();
    qint64 id = idAf(klient.sidsteSendtForTest());
    QVERIFY(!klient.hadRestoredHistory());
    QJsonObject resultat;
    resultat.insert("sessionId", "sess-gammel");
    klient.feedBytesForTest(svarJson(id, resultat) + "\n");
    QVERIFY(klient.hadRestoredHistory());
    QCOMPARE(klient.sessionId(), QString("sess-gammel"));
}

void TestGenoptagelseAfSamtale::resumeregel_laeser_manifest_og_finder_foerste_false()
{
    QString sti = QCoreApplication::applicationDirPath()
                  + "/../realiseringsmanifest_for_funktioner_og_parrede_tests.json";
    if (!QFileInfo::exists(sti))
        sti = "../realiseringsmanifest_for_funktioner_og_parrede_tests.json";
    QVERIFY2(QFileInfo::exists(sti), qPrintable(sti));
    QFile f(sti);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QJsonObject manifest = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    QJsonArray rækkefølge = manifest.value("strategisk_rækkefølge").toArray();
    QJsonArray funktioner = manifest.value("funktioner").toArray();
    QCOMPARE(funktioner.size(), rækkefølge.size());
    QHash<QString, bool> passerer;
    for (int i = 0; i < funktioner.size(); ++i)
        passerer.insert(funktioner.at(i).toObject().value("id").toString(),
                        funktioner.at(i).toObject().value("passerer").toBool());
    QString førsteIRækkefølge;
    for (int i = 0; i < rækkefølge.size(); ++i) {
        QString id = rækkefølge.at(i).toString();
        if (!passerer.value(id)) {
            førsteIRækkefølge = id;
            break;
        }
    }
    QString førsteIFunktioner;
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        if (!fn.value("passerer").toBool()) {
            førsteIFunktioner = fn.value("id").toString();
            break;
        }
    }
    QCOMPARE(førsteIRækkefølge, førsteIFunktioner);
}

#include "tst_genoptagelse_af_samtale.moc"
