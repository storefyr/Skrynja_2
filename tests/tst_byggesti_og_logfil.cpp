#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "logfile.h"
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
    void src_filnavne_matcher_klassenavne();
private:
    QString readLines(int& count);
    QString m_path;
};

REGISTER_TEST(TestByggestiOgLogfil)

void TestByggestiOgLogfil::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_path = QDir(QDir::tempPath() + "/skrynja-tst-logfil").filePath("skrynja-2.log");
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QFile::remove(m_path);
    LogFile::instance()->setLogPathForTest(m_path);
}

QString TestByggestiOgLogfil::readLines(int& count)
{
    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QString contents = QString::fromUtf8(f.readAll());
    f.close();
    count = contents.split('\n', QString::SkipEmptyParts).size();
    return contents;
}

void TestByggestiOgLogfil::logfil_oprettes_ved_foerste_linje()
{
    QFile::remove(m_path);
    QVERIFY(LogFile::instance()->writeLine("start"));
    QVERIFY(QFileInfo::exists(m_path));
    int count = 0;
    readLines(count);
    QCOMPARE(count, 1);
}

void TestByggestiOgLogfil::logfil_linje_har_iso_tidsstempel()
{
    QFile::remove(m_path);
    QVERIFY(LogFile::instance()->writeLine("start"));
    int count = 0;
    QString contents = readLines(count);
    QCOMPARE(count, 1);
    QRegExp iso("^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}.* start$");
    QVERIFY2(iso.exactMatch(contents.trimmed()), qPrintable(contents));
}

void TestByggestiOgLogfil::logfil_appender_uden_at_overskrive()
{
    QFile::remove(m_path);
    QVERIFY(LogFile::instance()->writeLine("start"));
    QVERIFY(LogFile::instance()->writeLine("mere"));
    int count = 0;
    readLines(count);
    QCOMPARE(count, 2);
}

void TestByggestiOgLogfil::logfil_roerer_ikke_transskripter()
{
    QFile::remove(m_path);
    QVERIFY(LogFile::instance()->writeLine("start"));
    int before = 0;
    readLines(before);
    QString transskript = "agent siger noget\nhemmelig samtale-linje\nendnu en linje";
    QVERIFY(!LogFile::instance()->writeLine(transskript));
    int after = 0;
    QString contents = readLines(after);
    QCOMPARE(after, before);
    QVERIFY(!contents.contains("hemmelig samtale-linje"));
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("harbour");
    QCoreApplication::setApplicationName("tst_skrynja_2");
    int failed = 0;
    QStringList navne;
    for (int i = 0; i < testRegistrations().size(); ++i) {
        QObject* test = testRegistrations().at(i).factory();
        navne.append(testRegistrations().at(i).name);
        failed += QTest::qExec(test, argc, argv);
        delete test;
    }
    if (failed == 0)
        qDebug("ALL TESTS PASSED (heraf %s)", qPrintable(navne.join(", ")));
    else
        qDebug("TESTS FEJLEDE: %d fejlede i %s", failed, qPrintable(navne.join(", ")));
    return failed;
}

void TestByggestiOgLogfil::src_filnavne_matcher_klassenavne()
{
    QDir srcDir(testRepoRoot() + "/src");
    QStringList filer = srcDir.entryList(QStringList() << "*.h" << "*.cpp", QDir::Files);
    for (int i = 0; i < filer.size(); ++i)
        QVERIFY2(filer.at(i).toUtf8().size() == filer.at(i).size(),
                 qPrintable("ikke-ascii filnavn: " + filer.at(i)));

    const char* forventede[] = {
        "logfile", "acpframereader", "acpmessage", "agentprocess", "acpprotokol",
        "acpsession", "acpclient", "conversationmodel", "sessionstore", "toolcallmodel",
        "permissionmodel", "diffmodel", "configurationmodel", "selftest", "main"
    };
    for (int i = 0; i < 15; ++i) {
        QString base = QString::fromLatin1(forventede[i]);
        if (base != "main")
            QVERIFY2(QFileInfo::exists(srcDir.filePath(base + ".h")), qPrintable(base + ".h mangler"));
        QVERIFY2(QFileInfo::exists(srcDir.filePath(base + ".cpp")), qPrintable(base + ".cpp mangler"));
    }
}

#include "tst_byggesti_og_logfil.moc"
