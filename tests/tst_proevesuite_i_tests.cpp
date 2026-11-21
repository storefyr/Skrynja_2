#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QRegExp>

#include "testregistrering.h"

class TestProevesuiteITests : public QObject
{
    Q_OBJECT
private slots:
    void manifest_har_13_parrede_tests();
    void alle_manifest_testfiler_er_i_tests_pro();
    void ingen_silica_afhaengighed_i_tests_pro();
    void suite_omfatter_alle_13_testklasser();
    void faelles_hjaelpere_i_testregistrering_h();
    void ingen_dublerede_slots_paa_tvaers();
    void ingen_global_settings_mutation_i_tests();
private:
    QJsonObject readManifest();
    QString readTestsPro();
};

REGISTER_TEST(TestProevesuiteITests)

QJsonObject TestProevesuiteITests::readManifest()
{
    QFile f(testRepoRoot() + "/realiseringsmanifest_for_funktioner_og_parrede_tests.json");
    if (!f.open(QIODevice::ReadOnly))
        return QJsonObject();
    return QJsonDocument::fromJson(f.readAll()).object();
}

QString TestProevesuiteITests::readTestsPro()
{
    QString path = QCoreApplication::applicationDirPath() + "/tests.pro";
    if (!QFileInfo::exists(path))
        path = "tests.pro";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(f.readAll());
}

void TestProevesuiteITests::manifest_har_13_parrede_tests()
{
    QJsonObject manifest = readManifest();
    QVERIFY(!manifest.isEmpty());
    QJsonArray funktioner = manifest.value("funktioner").toArray();
    QCOMPARE(funktioner.size(), 16);
    QCOMPARE(manifest.value("strategisk_rækkefølge").toArray().size(), 16);
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        QJsonObject pt = fn.value("parret_test").toObject();
        QVERIFY2(!pt.value("testfil").toString().isEmpty(), qPrintable(fn.value("id").toString()));
        QVERIFY2(!pt.value("testklasse").toString().isEmpty(), qPrintable(fn.value("id").toString()));
        QVERIFY2(!pt.value("testslots").toArray().isEmpty(), qPrintable(fn.value("id").toString()));
    }
}

void TestProevesuiteITests::alle_manifest_testfiler_er_i_tests_pro()
{
    QJsonObject manifest = readManifest();
    QString testsProContent = readTestsPro();
    QVERIFY(!testsProContent.isEmpty());
    QJsonArray funktioner = manifest.value("funktioner").toArray();
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        if (!fn.value("passerer").toBool())
            continue;
        QJsonObject pt = fn.value("parret_test").toObject();
        QString testfil = pt.value("testfil").toString();
        QVERIFY2(QFileInfo::exists(testRepoRoot() + "/" + testfil), qPrintable(testfil));
        QVERIFY2(testsProContent.contains(QFileInfo(testfil).fileName()),
                 qPrintable("ikke i tests.pro: " + testfil));
    }
}

void TestProevesuiteITests::ingen_silica_afhaengighed_i_tests_pro()
{
    QString testsProContent = readTestsPro();
    QVERIFY(testsProContent.contains("testlib"));
    QVERIFY(!testsProContent.contains("silica", Qt::CaseInsensitive));
    QVERIFY(!testsProContent.contains("Sailfish", Qt::CaseInsensitive));
}

void TestProevesuiteITests::suite_omfatter_alle_13_testklasser()
{
    QJsonObject manifest = readManifest();
    QJsonArray funktioner = manifest.value("funktioner").toArray();
    QStringList registrerede;
    for (int i = 0; i < testRegistrations().size(); ++i)
        registrerede.append(testRegistrations().at(i).name);
    int forventet = 0;
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        QString testfil = fn.value("parret_test").toObject().value("testfil").toString();
        if (QFileInfo::exists(testRepoRoot() + "/" + testfil)) {
            ++forventet;
            QString klasse = fn.value("parret_test").toObject().value("testklasse").toString();
            QVERIFY2(registrerede.contains(klasse), qPrintable(klasse));
        }
    }
    QCOMPARE(registrerede.size(), forventet);
}

void TestProevesuiteITests::faelles_hjaelpere_i_testregistrering_h()
{
    QFile f(testRepoRoot() + "/tests/testregistrering.h");
    QVERIFY(f.open(QIODevice::ReadOnly));
    QString h = QString::fromUtf8(f.readAll());
    QVERIFY(h.contains("opretTestSettingsSti"));
    QVERIFY(h.contains("testJsonObj"));
    QVERIFY(h.contains("testIdOf"));
    QVERIFY(h.contains("testResponseJson"));
    QVERIFY(h.contains("testErrorResponseJson"));
    QVERIFY(h.contains("testReadFixture"));

    QJsonObject r = testJsonObj(testResponseJson(7, QJsonObject()));
    QCOMPARE(testIdOf(testResponseJson(7, QJsonObject())), qint64(7));
    QCOMPARE(r.value("id").toVariant().toLongLong(), qint64(7));
    QVERIFY(testErrorResponseJson(8, -1, "x").contains("error"));
    QVERIFY(!testReadFixture("fixture_rygrad.ndjson").isEmpty());
}

void TestProevesuiteITests::ingen_dublerede_slots_paa_tvaers()
{
    QDir dir(testRepoRoot() + "/tests");
    QStringList filer = dir.entryList(QStringList() << "tst_*.cpp", QDir::Files);
    QVERIFY(filer.size() >= 16);

    QJsonObject manifest = readManifest();
    QJsonArray funktioner = manifest.value("funktioner").toArray();
    QMap<QString, int> manifestLaaste;
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        if (!fn.value("passerer").toBool())
            continue;
        QJsonArray slotListe = fn.value("parret_test").toObject().value("testslots").toArray();
        for (int j = 0; j < slotListe.size(); ++j)
            manifestLaaste[slotListe.at(j).toString()] += 1;
    }

    QMap<QString, QStringList> prFil;
    for (int i = 0; i < filer.size(); ++i) {
        QFile f(dir.filePath(filer.at(i)));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QString src = QString::fromUtf8(f.readAll());
        QRegExp defRe("void Test\\w+::(\\w+)\\(");
        int pos = 0;
        while ((pos = defRe.indexIn(src, pos)) != -1) {
            QString slot = defRe.cap(1);
            if (slot != "initTestCase" && slot != "cleanupTestCase")
                prFil[slot].append(filer.at(i));
            pos += defRe.matchedLength();
        }
    }
    for (QMap<QString, QStringList>::const_iterator it = prFil.constBegin(); it != prFil.constEnd(); ++it) {
        if (manifestLaaste.value(it.key()) > 1)
            continue;
        QVERIFY2(it.value().size() == 1,
                 qPrintable("dubleret slot " + it.key() + " i " + it.value().join(",")));
    }
}

void TestProevesuiteITests::ingen_global_settings_mutation_i_tests()
{
    QDir dir(testRepoRoot() + "/tests");
    QStringList filer = dir.entryList(QStringList() << "tst_*.cpp", QDir::Files);
    QVERIFY(filer.size() >= 16);
    for (int i = 0; i < filer.size(); ++i) {
        QString src = testReadSource("tests/" + filer.at(i));
        QVERIFY2(!src.contains("QSettings::setPath(QSettings"),
                 qPrintable("direkte global QSettings::setPath i " + filer.at(i)));
        QVERIFY2(!src.contains("QSettings::setDefaultFormat(QSettings"),
                 qPrintable("direkte global QSettings::setDefaultFormat i " + filer.at(i)));
        if (src.contains("QSettings") && src.contains("void") && src.contains("initTestCase"))
            QVERIFY2(src.contains("opretTestSettingsSti"),
                     qPrintable("initTestCase uden opretTestSettingsSti i " + filer.at(i)));
    }
    QString h = testReadSource("tests/testregistrering.h");
    QVERIFY(h.contains("QSettings::setPath"));
}

#include "tst_proevesuite_i_tests.moc"
