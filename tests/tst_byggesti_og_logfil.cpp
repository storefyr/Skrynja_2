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
    void manglende_qml_giver_ikke_exit_0();
    void startAgent_fejl_signaleres();
    void ikoner_findes_i_fire_stoerrelser_og_er_png();
    void ikoner_wired_i_pro_spec_og_yaml();
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
    LogFile::instance()->setLogPath(m_path);
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

void TestByggestiOgLogfil::manglende_qml_giver_ikke_exit_0()
{
    QString main = testReadSource("src/main.cpp");
    QVERIFY(!main.isEmpty());
    QVERIFY2(main.contains("return 2"), "main.cpp giver ikke exit != 0 ved manglende QML-fil");
    QVERIFY2(main.contains("qWarning"), "main.cpp skriver ikke besked ved manglende QML-fil");
}

void TestByggestiOgLogfil::startAgent_fejl_signaleres()
{
    QString main = testReadSource("src/main.cpp");
    QVERIFY(!main.isEmpty());
    QVERIFY2(main.contains("if (!client.startAgent())"), "main.cpp tjekker ikke startAgent()-resultatet");
    QVERIFY2(main.contains("return 3"), "main.cpp giver ikke exit != 0 ved startAgent-fejl");
}

void TestByggestiOgLogfil::ikoner_findes_i_fire_stoerrelser_og_er_png()
{
    const char* stoerrelser[] = {"86x86", "108x108", "128x128", "172x172"};
    for (int i = 0; i < 4; ++i) {
        QString path = testRepoRoot() + "/icons/" + stoerrelser[i] + "/harbour-skrynja-2.png";
        QVERIFY2(QFileInfo::exists(path), qPrintable("mangler " + path));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray head = f.read(24);
        f.close();
        QCOMPARE(head.size(), 24);
        QVERIFY2(head.startsWith(QByteArray("\x89PNG\r\n\x1a\n")), qPrintable("ikke PNG: " + path));
        QCOMPARE(QString::fromLatin1(head.mid(12, 4)), QString("IHDR"));
        int w = ((quint8)head.at(16) << 24) | ((quint8)head.at(17) << 16) | ((quint8)head.at(18) << 8) | (quint8)head.at(19);
        int h = ((quint8)head.at(20) << 24) | ((quint8)head.at(21) << 16) | ((quint8)head.at(22) << 8) | (quint8)head.at(23);
        int forventet = QString(stoerrelser[i]).split('x').at(0).toInt();
        QCOMPARE(w, forventet);
        QCOMPARE(h, forventet);
    }
}

void TestByggestiOgLogfil::ikoner_wired_i_pro_spec_og_yaml()
{
    QString pro = testReadSource("harbour-skrynja-2.pro");
    QVERIFY2(!pro.isEmpty(), "harbour-skrynja-2.pro mangler");
    const char* stoerrelser[] = {"86x86", "108x108", "128x128", "172x172"};
    for (int i = 0; i < 4; ++i) {
        QString s = stoerrelser[i];
        QVERIFY2(pro.contains("icons/" + s + "/harbour-skrynja-2.png"),
                 qPrintable("pro mangler ikonfil " + s));
        QVERIFY2(pro.contains("/usr/share/icons/hicolor/" + s + "/apps"),
                 qPrintable("pro mangler install-path for " + s));
    }

    QString spec = testReadSource("rpm/harbour-skrynja-2.spec");
    QVERIFY2(!spec.isEmpty(), "spec mangler");
    QVERIFY2(spec.contains("%{_datadir}/icons/hicolor"), "spec mangler icons-hicolor i %files");
    QVERIFY2(spec.contains("%{name}.png"), "spec mangler %{name}.png");

    QString yaml = testReadSource("harbour-skrynja-2.yaml");
    QVERIFY2(!yaml.isEmpty(), "yaml mangler");
    QVERIFY2(yaml.contains("icons"), "yaml Files mangler icons");

    QString desktop = testReadSource("harbour-skrynja-2.desktop");
    QVERIFY2(desktop.contains("Icon=harbour-skrynja-2"), "desktop skal beholde Icon=harbour-skrynja-2");
}

#include "tst_byggesti_og_logfil.moc"
