#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>
#include <QStandardPaths>

#include "agentprocess.h"
#include "logfile.h"
#include "testregistrering.h"

class TestOprydningAfTrae : public QObject
{
    Q_OBJECT
private slots:
    void git_status_porcelain_tom_efter_indskrivning();
    void ingen_hemmeligheder_i_trae();
    void ingen_build_artefakter_i_trae();
    void agentproces_ikke_haengende();
    void logfil_uden_transskripter();
    void status_test_er_reproducerbar();
    void ingen_pgrep_mod_vaertsmiljoe();
private:
    QString runGit(const QStringList& argumenter);
};

REGISTER_TEST(TestOprydningAfTrae)

QString TestOprydningAfTrae::runGit(const QStringList& argumenter)
{
    QProcess p;
    p.setWorkingDirectory(testRepoRoot());
    p.start("git", argumenter);
    p.waitForFinished(15000);
    return QString::fromUtf8(p.readAllStandardOutput());
}

void TestOprydningAfTrae::git_status_porcelain_tom_efter_indskrivning()
{
    QString output = runGit(QStringList() << "status" << "--porcelain" << "--"
                           << "src" << "tests" << "qml" << "harbour-skrynja-2.pro"
                           << "harbour-skrynja-2.desktop" << "harbour-skrynja-2.yaml" << "rpm"
                           << "realiseringsmanifest_for_funktioner_og_parrede_tests.json"
                           << "projektbeskrivelse.adoc" << "tjek_manifest_parrede_tests.py"
                           << ".gitignore");
    QVERIFY2(output.trimmed().isEmpty(), qPrintable(output));
}

void TestOprydningAfTrae::ingen_hemmeligheder_i_trae()
{
    QString pattern = "api[_-]?key|tok" "en|pass" "word|BEGIN [A-Z ]*PRIVATE KEY";
    QString output = runGit(QStringList() << "grep" << "-nE" << pattern << "--" << "." << ":!LICENSE");
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    QStringList allowed;
    allowed << "realiseringsmanifest_for_funktioner_og_parrede_tests.json"
             << "projektbeskrivelse.adoc";
    for (int i = 0; i < lines.size(); ++i) {
        QString file = lines.at(i).section(':', 0, 0);
        QVERIFY2(allowed.contains(file),
                 qPrintable("uventet hit (mulig hemmelighed): " + lines.at(i)));
    }
}

void TestOprydningAfTrae::ingen_build_artefakter_i_trae()
{
    QString output = runGit(QStringList() << "ls-files");
    QRegExp artefakt("(^|/)(Makefile|\\.o$|moc_|qrc_|ui_|\\.qmake\\.stash|proeve_skrynja_2|harbour-skrynja-2)$");
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    for (int i = 0; i < lines.size(); ++i)
        QVERIFY2(!artefakt.exactMatch(lines.at(i)), qPrintable("build-artefakt i træet: " + lines.at(i)));
}

void TestOprydningAfTrae::agentproces_ikke_haengende()
{
    AgentProcess p("/bin/sleep", QStringList() << "30");
    QVERIFY(p.start());
    QVERIFY(p.isRunning());
    QVERIFY(p.stopAgent());
    QVERIFY(!p.isRunning());
}

void TestOprydningAfTrae::logfil_uden_transskripter()
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + "/skrynja-tst-logfil-oprydning/skrynja-2.log";
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile::remove(path);
    LogFile::instance()->setLogPath(path);
    QVERIFY(LogFile::instance()->writeLine("start"));
    QVERIFY(!LogFile::instance()->writeLine("transskript\nmed flere linjer"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString contents = QString::fromUtf8(f.readAll());
    f.close();
    QCOMPARE(contents.split('\n', QString::SkipEmptyParts).size(), 1);
    QVERIFY(!contents.contains("transskript"));
}

void TestOprydningAfTrae::status_test_er_reproducerbar()
{
    QString kilde = testReadSource("tests/proeve_oprydning_af_trae.cpp");
    QVERIFY2(kilde.contains("--porcelain"), "git status uden --porcelain");
    QVERIFY2(kilde.contains("-- \"src\"") || kilde.contains("<< \"src\""),
             "git status tæller ikke kun udvalgte sporede filer");
    QString output = runGit(QStringList() << "status" << "--porcelain" << "--"
                           << "src" << "tests" << "qml" << "harbour-skrynja-2.pro"
                           << "realiseringsmanifest_for_funktioner_og_parrede_tests.json");
    QVERIFY2(output.trimmed().isEmpty(), qPrintable(output));
}

void TestOprydningAfTrae::ingen_pgrep_mod_vaertsmiljoe()
{
    QString kilde = testReadSource("tests/proeve_oprydning_af_trae.cpp");
    QVERIFY2(!kilde.contains("\"pgrep\""), "testen pgrep'er stadig værtsmiljøet");
}

#include "proeve_oprydning_af_trae.moc"
