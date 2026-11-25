#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QRegExp>
#include <QProcess>

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
    void meta_test_afledt_af_manifest_ikke_hardkodet();
    void ingen_testkroge_i_produktionsklasser();
    void plan_og_manifest_spejler_hinanden();
    void ingen_afkortede_proeve_filnavne_i_tests();
    void spejlkontrol_daekker_navngivningsregel();
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
    QCOMPARE(funktioner.size(), manifest.value("strategisk_rækkefølge").toArray().size());
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
    QStringList klaser;
    int forventet = 0;
    for (int i = 0; i < funktioner.size(); ++i) {
        QJsonObject fn = funktioner.at(i).toObject();
        if (!fn.value("passerer").toBool())
            continue;
        QString testfil = fn.value("parret_test").toObject().value("testfil").toString();
        if (QFileInfo::exists(testRepoRoot() + "/" + testfil)) {
            QString klasse = fn.value("parret_test").toObject().value("testklasse").toString();
            QVERIFY2(registrerede.contains(klasse), qPrintable(klasse));
            if (!klaser.contains(klasse)) {
                klaser.append(klasse);
                ++forventet;
            }
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
    QStringList filer = dir.entryList(QStringList() << "proeve_*.cpp", QDir::Files);
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
    QStringList filer = dir.entryList(QStringList() << "proeve_*.cpp", QDir::Files);
    QVERIFY(filer.size() >= 16);
    for (int i = 0; i < filer.size(); ++i) {
        QString src = testReadSource("tests/" + filer.at(i));
        if (filer.at(i) != "proeve_proevesuite_i_tests.cpp") {
            QVERIFY2(!src.contains("QSettings::setPath(QSettings"),
                     qPrintable("direkte global QSettings::setPath i " + filer.at(i)));
            QVERIFY2(!src.contains("QSettings::setDefaultFormat(QSettings"),
                     qPrintable("direkte global QSettings::setDefaultFormat i " + filer.at(i)));
        }
        if (src.contains("QSettings") && src.contains("void") && src.contains("initTestCase"))
            QVERIFY2(src.contains("opretTestSettingsSti"),
                     qPrintable("initTestCase uden opretTestSettingsSti i " + filer.at(i)));
    }
    QString h = testReadSource("tests/testregistrering.h");
    QVERIFY(h.contains("QSettings::setPath"));
}


void TestProevesuiteITests::meta_test_afledt_af_manifest_ikke_hardkodet()
{
    QString kilde = testReadSource("tests/proeve_proevesuite_i_tests.cpp");
    QVERIFY(!kilde.isEmpty());
    int pos = kilde.indexOf("meta_test_afledt_af_manifest_ikke_hardkodet()\n{");
    QVERIFY(pos >= 0);
    QString udenEgenSlot = kilde.left(pos);
    QVERIFY2(!udenEgenSlot.contains("size(), 13)"), "haardkodet funktionsantal 13 i meta-test");
    QVERIFY2(!udenEgenSlot.contains("size(), 16)"), "haardkodet funktionsantal 16 i meta-test");
    QVERIFY2(kilde.contains("strategisk_r"), "meta-test aflaeser ikke manifestets raekkefoelge");
}

void TestProevesuiteITests::ingen_testkroge_i_produktionsklasser()
{
    QDir dir(testRepoRoot() + "/src");
    QStringList filer = dir.entryList(QStringList() << "*.h" << "*.cpp", QDir::Files);
    QVERIFY(filer.size() >= 15);
    for (int i = 0; i < filer.size(); ++i) {
        QString src = testReadSource("src/" + filer.at(i));
        QVERIFY2(!src.contains("ForTest"), qPrintable("ForTest-krog i " + filer.at(i)));
        QVERIFY2(!src.contains("m_testMode"), qPrintable("m_testMode i " + filer.at(i)));
    }
}

void TestProevesuiteITests::plan_og_manifest_spejler_hinanden()
{
    QString script = testRepoRoot() + "/tjek_plan_manifest_spejl.py";
    QVERIFY2(QFileInfo::exists(script), "tjek_plan_manifest_spejl.py mangler");

    QProcess prosess;
    prosess.setWorkingDirectory(testRepoRoot());
    prosess.start("python3", QStringList() << script);
    QVERIFY(prosess.waitForFinished(30000));
    QCOMPARE(prosess.exitCode(), 0);
    QString ud = QString::fromUtf8(prosess.readAllStandardOutput()) + QString::fromUtf8(prosess.readAllStandardError());
    QVERIFY2(ud.contains("spejl ok"), qPrintable(ud));

    QJsonObject manifest = readManifest();
    QJsonArray raekkefoelge = manifest.value("strategisk_rækkefølge").toArray();
    QVERIFY(raekkefoelge.size() > 1);
    QJsonArray brudt = raekkefoelge;
    brudt.removeLast();
    manifest.insert("strategisk_rækkefølge", brudt);
    QString tempSti = QDir::tempPath() + "/skrynja-tst-spejl-brudt-manifest.json";
    QFile temp(tempSti);
    QVERIFY(temp.open(QIODevice::WriteOnly));
    temp.write(QJsonDocument(manifest).toJson());
    temp.close();

    QProcess negativ;
    negativ.setWorkingDirectory(testRepoRoot());
    negativ.start("python3", QStringList() << script << "--manifest" << tempSti);
    QVERIFY(negativ.waitForFinished(30000));
    QVERIFY(negativ.exitCode() != 0);
    QString negativUd = QString::fromUtf8(negativ.readAllStandardOutput()) + QString::fromUtf8(negativ.readAllStandardError());
    QVERIFY2(negativUd.contains("FEJL"), qPrintable(negativUd));
    QFile::remove(tempSti);
}

void TestProevesuiteITests::spejlkontrol_daekker_navngivningsregel()
{
    QString script = testRepoRoot() + "/tjek_plan_manifest_spejl.py";
    QVERIFY2(QFileInfo::exists(script), "tjek_plan_manifest_spejl.py mangler");
    QFile scriptFil(script);
    QVERIFY2(scriptFil.open(QIODevice::ReadOnly), "kan ikke læse tjek_plan_manifest_spejl.py");
    QString scriptTekst = QString::fromUtf8(scriptFil.readAll());
    scriptFil.close();
    QVERIFY2(scriptTekst.contains("Navngivning"), "NOEGLEREGLER i tjek_plan_manifest_spejl.py dækker ikke navngivningsreglen");

    QJsonObject manifest = readManifest();
    QVERIFY(!manifest.isEmpty());
    QJsonArray arbejdsregler = manifest.value("arbejdsregler").toArray();
    QJsonArray udenNavngivning;
    bool harNavngivning = false;
    for (int i = 0; i < arbejdsregler.size(); ++i) {
        QString regel = arbejdsregler.at(i).toString();
        if (regel.contains("Navngivning"))
            harNavngivning = true;
        else
            udenNavngivning.append(regel);
    }
    QVERIFY2(harNavngivning, "manifestets arbejdsregler mangler navngivningsreglen");
    manifest.insert("arbejdsregler", udenNavngivning);

    QString tempSti = QDir::tempPath() + "/skrynja-tst-spejl-uden-navngivning.json";
    QFile temp(tempSti);
    QVERIFY(temp.open(QIODevice::WriteOnly));
    temp.write(QJsonDocument(manifest).toJson());
    temp.close();

    QProcess negativ;
    negativ.setWorkingDirectory(testRepoRoot());
    negativ.start("python3", QStringList() << script << "--manifest" << tempSti);
    QVERIFY(negativ.waitForFinished(30000));
    QVERIFY(negativ.exitCode() != 0);
    QString negativUd = QString::fromUtf8(negativ.readAllStandardOutput()) + QString::fromUtf8(negativ.readAllStandardError());
    QVERIFY2(negativUd.contains("Navngivning"), qPrintable(negativUd));
    QFile::remove(tempSti);
}

void TestProevesuiteITests::ingen_afkortede_proeve_filnavne_i_tests()
{
    QDir dir(testRepoRoot() + "/tests");
    QStringList filer = dir.entryList(QStringList() << "*.cpp" << "*.h", QDir::Files);
    int proeveAntal = 0;
    for (int i = 0; i < filer.size(); ++i) {
        QVERIFY2(!filer.at(i).startsWith("proeve_"), qPrintable("afkortet filnavn: " + filer.at(i)));
        if (filer.at(i).startsWith("proeve_"))
            ++proeveAntal;
        QString src = testReadSource("tests/" + filer.at(i));
        QVERIFY2(!src.contains("#include \"proeve_"), qPrintable("proeve_-moc-include i " + filer.at(i)));
    }
    QVERIFY2(proeveAntal >= 16, qPrintable("for faa proeve_*-filer: " + QString::number(proeveAntal)));
    QVERIFY2(!testReadSource("tests/tests.pro").contains("proeve_"), "tests.pro har stadig proeve_-referencer");
    QVERIFY2(!testReadSource(".gitignore").contains("proeve_"), ".gitignore har stadig proeve_-referencer");
}

#include "proeve_proevesuite_i_tests.moc"
