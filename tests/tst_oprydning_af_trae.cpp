#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>
#include <QStandardPaths>

#include "logfil.h"
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
private:
    QString repoRod();
    QString kørGit(const QStringList& argumenter);
};

REGISTRER_TEST(TestOprydningAfTrae)

QString TestOprydningAfTrae::repoRod()
{
    QString rod = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(rod + "/tjek_manifest_parrede_tests.py"))
        rod = "..";
    return rod;
}

QString TestOprydningAfTrae::kørGit(const QStringList& argumenter)
{
    QProcess p;
    p.setWorkingDirectory(repoRod());
    p.start("git", argumenter);
    p.waitForFinished(15000);
    return QString::fromUtf8(p.readAllStandardOutput());
}

void TestOprydningAfTrae::git_status_porcelain_tom_efter_indskrivning()
{
    QString ud = kørGit(QStringList() << "status" << "--porcelain" << "-uno");
    QVERIFY2(ud.trimmed().isEmpty(), qPrintable(ud));
}

void TestOprydningAfTrae::ingen_hemmeligheder_i_trae()
{
    QString mønster = "api[_-]?key|tok" "en|pass" "word|BEGIN [A-Z ]*PRIVATE KEY";
    QString ud = kørGit(QStringList() << "grep" << "-nE" << mønster << "--" << "." << ":!LICENSE");
    QStringList linjer = ud.split('\n', QString::SkipEmptyParts);
    QStringList tilladte;
    tilladte << "realiseringsmanifest_for_funktioner_og_parrede_tests.json"
             << "projektbeskrivelse.adoc";
    for (int i = 0; i < linjer.size(); ++i) {
        QString fil = linjer.at(i).section(':', 0, 0);
        QVERIFY2(tilladte.contains(fil),
                 qPrintable("uventet hit (mulig hemmelighed): " + linjer.at(i)));
    }
}

void TestOprydningAfTrae::ingen_build_artefakter_i_trae()
{
    QString ud = kørGit(QStringList() << "ls-files");
    QRegExp artefakt("(^|/)(Makefile|\\.o$|moc_|qrc_|ui_|\\.qmake\\.stash|tst_skrynja_2|harbour-skrynja-2)$");
    QStringList linjer = ud.split('\n', QString::SkipEmptyParts);
    for (int i = 0; i < linjer.size(); ++i)
        QVERIFY2(!artefakt.exactMatch(linjer.at(i)), qPrintable("build-artefakt i træet: " + linjer.at(i)));
}

void TestOprydningAfTrae::agentproces_ikke_haengende()
{
    QProcess p;
    p.start("pgrep", QStringList() << "-f" << "vibe-acp");
    QVERIFY(p.waitForFinished(5000));
    QString ud = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
    QVERIFY2(ud.isEmpty(), qPrintable("haengende agentproces: " + ud));
}

void TestOprydningAfTrae::logfil_uden_transskripter()
{
    QStandardPaths::setTestModeEnabled(true);
    QString sti = QDir::tempPath() + "/skrynja-tst-logfil-oprydning/skrynja-2.log";
    QDir().mkpath(QFileInfo(sti).absolutePath());
    QFile::remove(sti);
    Logfil::instans()->sætLogstiForTest(sti);
    QVERIFY(Logfil::instans()->skrivLinje("start"));
    QVERIFY(!Logfil::instans()->skrivLinje("transskript\nmed flere linjer"));
    QFile f(sti);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    QString indhold = QString::fromUtf8(f.readAll());
    f.close();
    QCOMPARE(indhold.split('\n', QString::SkipEmptyParts).size(), 1);
    QVERIFY(!indhold.contains("transskript"));
}

#include "tst_oprydning_af_trae.moc"
