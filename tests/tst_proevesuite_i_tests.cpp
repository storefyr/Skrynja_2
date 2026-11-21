#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "testregistrering.h"

class TestProevesuiteITests : public QObject
{
    Q_OBJECT
private slots:
    void manifest_har_13_parrede_tests();
    void alle_manifest_testfiler_er_i_tests_pro();
    void ingen_silica_afhaengighed_i_tests_pro();
    void suite_omfatter_alle_13_testklasser();
private:
    QString repoRoot();
    QJsonObject readManifest();
    QString readTestsPro();
};

REGISTER_TEST(TestProevesuiteITests)

QString TestProevesuiteITests::repoRoot()
{
    QString root = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(root + "/realiseringsmanifest_for_funktioner_og_parrede_tests.json"))
        root = "..";
    return root;
}

QJsonObject TestProevesuiteITests::readManifest()
{
    QFile f(repoRoot() + "/realiseringsmanifest_for_funktioner_og_parrede_tests.json");
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
    QCOMPARE(funktioner.size(), 13);
    QCOMPARE(manifest.value("strategisk_rækkefølge").toArray().size(), 13);
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
        QJsonObject pt = funktioner.at(i).toObject().value("parret_test").toObject();
        QString testfil = pt.value("testfil").toString();
        QVERIFY2(QFileInfo::exists(repoRoot() + "/" + testfil), qPrintable(testfil));
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
    QCOMPARE(registrerede.size(), 13);
    for (int i = 0; i < funktioner.size(); ++i) {
        QString klasse = funktioner.at(i).toObject().value("parret_test").toObject()
                             .value("testklasse").toString();
        QVERIFY2(registrerede.contains(klasse), qPrintable(klasse));
    }
}

#include "tst_proevesuite_i_tests.moc"
