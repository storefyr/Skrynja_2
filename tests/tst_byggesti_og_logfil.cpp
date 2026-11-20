#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "logfil.h"
#include "testregistrering.h"

class TestByggestiOgLogfil : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void logfil_oprettes_ved_foerste_linje();
    void logfil_linje_har_iso_tidsstempel();
    void logfil_appender_uden_at_overskrive();
    void logfil_roerer_ikke_transskripter();
private:
    QString lesLinjer(int& antal);
    QString m_sti;
};

REGISTRER_TEST(TestByggestiOgLogfil)

void TestByggestiOgLogfil::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_sti = QDir(QDir::tempPath() + "/skrynja-tst-logfil").filePath("skrynja-2.log");
    QDir().mkpath(QFileInfo(m_sti).absolutePath());
    QFile::remove(m_sti);
    Logfil::instans()->sætLogstiForTest(m_sti);
}

QString TestByggestiOgLogfil::lesLinjer(int& antal)
{
    QFile f(m_sti);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QString indhold = QString::fromUtf8(f.readAll());
    f.close();
    antal = indhold.split('\n', QString::SkipEmptyParts).size();
    return indhold;
}

void TestByggestiOgLogfil::logfil_oprettes_ved_foerste_linje()
{
    QFile::remove(m_sti);
    QVERIFY(Logfil::instans()->skrivLinje("start"));
    QVERIFY(QFileInfo::exists(m_sti));
    int antal = 0;
    lesLinjer(antal);
    QCOMPARE(antal, 1);
}

void TestByggestiOgLogfil::logfil_linje_har_iso_tidsstempel()
{
    QFile::remove(m_sti);
    QVERIFY(Logfil::instans()->skrivLinje("start"));
    int antal = 0;
    QString indhold = lesLinjer(antal);
    QCOMPARE(antal, 1);
    QRegExp iso("^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}.* start$");
    QVERIFY2(iso.exactMatch(indhold.trimmed()), qPrintable(indhold));
}

void TestByggestiOgLogfil::logfil_appender_uden_at_overskrive()
{
    QFile::remove(m_sti);
    QVERIFY(Logfil::instans()->skrivLinje("start"));
    QVERIFY(Logfil::instans()->skrivLinje("mere"));
    int antal = 0;
    lesLinjer(antal);
    QCOMPARE(antal, 2);
}

void TestByggestiOgLogfil::logfil_roerer_ikke_transskripter()
{
    QFile::remove(m_sti);
    QVERIFY(Logfil::instans()->skrivLinje("start"));
    int før = 0;
    lesLinjer(før);
    QString transskript = "agent siger noget\nhemmelig samtale-linje\nendnu en linje";
    QVERIFY(!Logfil::instans()->skrivLinje(transskript));
    int efter = 0;
    QString indhold = lesLinjer(efter);
    QCOMPARE(efter, før);
    QVERIFY(!indhold.contains("hemmelig samtale-linje"));
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("harbour");
    QCoreApplication::setApplicationName("tst_skrynja_2");
    int fejlede = 0;
    QStringList navne;
    for (int i = 0; i < testRegistreringer().size(); ++i) {
        QObject* test = testRegistreringer().at(i).fabrik();
        navne.append(testRegistreringer().at(i).navn);
        fejlede += QTest::qExec(test, argc, argv);
        delete test;
    }
    if (fejlede == 0)
        qDebug("ALL TESTS PASSED (heraf %s)", qPrintable(navne.join(", ")));
    else
        qDebug("TESTS FEJLEDE: %d fejlede i %s", fejlede, qPrintable(navne.join(", ")));
    return fejlede;
}

#include "tst_byggesti_og_logfil.moc"
