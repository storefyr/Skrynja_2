#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDirIterator>

#include "logfile.h"
#include "testregistrering.h"
#include <QProcess>

class TestByggestiOgLogfil : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void logfil_oprettes_ved_foerste_linje();
    void logfil_linje_har_iso_tidsstempel();
    void logfil_appender_uden_at_overskrive();
    void logfil_roerer_ikke_transskripter();
    void kilder_filnavne_matcher_klassenavne();
    void manglende_qml_giver_ikke_exit_0();
    void startAgent_fejl_signaleres();
    void ikoner_findes_i_fire_stoerrelser_og_er_png();
    void ikoner_wired_i_pro_spec_og_yaml();
    void spec_og_yaml_er_konsistente_med_pro();
    void ingen_rpm_eller_log_i_depotet();
    void release_workflow_bygger_og_udgiver_rpm();
    void verificeringsskript_paa_enhed_findes_og_bestaaer_bash_n();
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
    QCoreApplication::setApplicationName("proeve_skrynja_2");
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

void TestByggestiOgLogfil::kilder_filnavne_matcher_klassenavne()
{
    QVERIFY2(!QFileInfo::exists(testRepoRoot() + "/src"), "src/ findes stadig — mappen skal hedde kilder/");
    QDir kilderDir(testRepoRoot() + "/kilder");
    QVERIFY2(kilderDir.exists(), "kilder/ mangler");
    QStringList filer = kilderDir.entryList(QStringList() << "*.h" << "*.cpp", QDir::Files);
    for (int i = 0; i < filer.size(); ++i)
        QVERIFY2(filer.at(i).toUtf8().size() == filer.at(i).size(),
                 qPrintable("ikke-ascii filnavn: " + filer.at(i)));
    QVERIFY2(!testReadSource("harbour-skrynja-2.pro").contains("src/"), "harbour-skrynja-2.pro har stadig src/-stier");
    QVERIFY2(!testReadSource("tests/tests.pro").contains("../src"), "tests.pro har stadig ../src-stier");

    const char* forventede[] = {
        "logfile", "acpframereader", "acpmessage", "agentprocess", "acpprotokol",
        "acpsession", "acpclient", "conversationmodel", "sessionstore", "toolcallmodel",
        "permissionmodel", "diffmodel", "configurationmodel", "konfiguration", "selftest", "main"
    };
    for (int i = 0; i < 16; ++i) {
        QString base = QString::fromLatin1(forventede[i]);
        if (base != "main")
            QVERIFY2(QFileInfo::exists(kilderDir.filePath(base + ".h")), qPrintable(base + ".h mangler"));
        QVERIFY2(QFileInfo::exists(kilderDir.filePath(base + ".cpp")), qPrintable(base + ".cpp mangler"));
    }
}

void TestByggestiOgLogfil::manglende_qml_giver_ikke_exit_0()
{
    QString main = testReadSource("kilder/main.cpp");
    QVERIFY(!main.isEmpty());
    QVERIFY2(main.contains("return 2"), "main.cpp giver ikke exit != 0 ved manglende QML-fil");
    QVERIFY2(main.contains("qWarning"), "main.cpp skriver ikke besked ved manglende QML-fil");
}

void TestByggestiOgLogfil::startAgent_fejl_signaleres()
{
    QString main = testReadSource("kilder/main.cpp");
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

void TestByggestiOgLogfil::spec_og_yaml_er_konsistente_med_pro()
{
    QString pro = testReadSource("harbour-skrynja-2.pro");
    QString spec = testReadSource("rpm/harbour-skrynja-2.spec");
    QString yaml = testReadSource("harbour-skrynja-2.yaml");
    QString desktop = testReadSource("harbour-skrynja-2.desktop");
    QVERIFY2(!pro.isEmpty() && !spec.isEmpty() && !yaml.isEmpty() && !desktop.isEmpty(), "packaging-fil mangler");

    QRegExp targetRe("TARGET\\s*=\\s*(\\w[\\w-]*)");
    QVERIFY(targetRe.indexIn(pro) != -1);
    QString target = targetRe.cap(1);

    QVERIFY2(spec.contains("Name: " + target), "spec Name matcher ikke pro TARGET");
    QVERIFY2(spec.contains("Source0: %{name}-%{version}.tar.gz"), "spec Source0 følger ikke name-version");
    QVERIFY2(spec.contains("/usr/bin/" + target), "spec %files mangler binær");
    QVERIFY2(spec.contains("/usr/share/applications/" + target + ".desktop"), "spec %files mangler desktop");
    QVERIFY2(spec.contains("/usr/share/" + target + "/qml"), "spec %files mangler qml");
    QVERIFY2(spec.contains("%{_datadir}/icons/hicolor"), "spec %files mangler ikoner");
    QVERIFY2(spec.contains("%{name}.png"), "spec %files mangler ikon-glob");
    QVERIFY2(spec.contains("BuildRequires: pkgconfig(Qt5Core)"), "spec mangler pkgconfig(Qt5Core) BuildRequires");
    QVERIFY2(spec.contains("BuildRequires: pkgconfig(Qt5Quick)"), "spec mangler pkgconfig(Qt5Quick) BuildRequires");
    QVERIFY2(spec.contains("%qmake5 " + target + ".pro"), "spec %build skal køre %qmake5 på pro-filen");

    QVERIFY2(yaml.contains("Builder: qmake5"), "yaml Builder skal være qmake5");
    QVERIFY2(yaml.contains("- " + target + ".desktop"), "yaml Files mangler desktop");
    QVERIFY2(yaml.contains("- qml"), "yaml Files mangler qml");
    QVERIFY2(yaml.contains("- icons"), "yaml Files mangler icons");

    QVERIFY2(desktop.contains("Exec=" + target), "desktop Exec matcher ikke target");
    QVERIFY2(desktop.contains("Icon=" + target), "desktop Icon matcher ikke target");

    const char* stoerrelser[] = {"86x86", "108x108", "128x128", "172x172"};
    for (int i = 0; i < 4; ++i) {
        QString s = stoerrelser[i];
        QVERIFY2(pro.contains("/usr/share/icons/hicolor/" + s + "/apps"),
                 qPrintable("pro mangler hicolor-path for " + s));
    }
}

void TestByggestiOgLogfil::ingen_rpm_eller_log_i_depotet()
{
    QString gitignore = testReadSource(".gitignore");
    QVERIFY2(gitignore.contains("*.rpm"), ".gitignore skal ignorere *.rpm");
    QVERIFY2(gitignore.contains("*.log"), ".gitignore skal ignorere *.log");

    QStringList forbudte;
    QDirIterator it(testRepoRoot(), QStringList() << "*.rpm" << "*.log", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (path.contains("/.git/"))
            continue;
        forbudte.append(path);
    }
    QVERIFY2(forbudte.isEmpty(), qPrintable("build-artefakter i depotet: " + forbudte.join(", ")));
}

void TestByggestiOgLogfil::release_workflow_bygger_og_udgiver_rpm()
{
    QString wf = testReadSource(".github/workflows/release.yml");
    QVERIFY2(!wf.isEmpty(), ".github/workflows/release.yml mangler");
    QVERIFY2(wf.contains("sailfish-build-rpm"), "workflow skal bygge mål-byg i Sailfish SDK (sailfish-build-rpm)");
    QVERIFY2(wf.contains("aarch64"), "workflow skal bygge aarch64");
    QVERIFY2(wf.contains("v*.*.*"), "workflow skal trigge på tags v*.*.*");
    QVERIFY2(wf.contains("workflow_dispatch"), "workflow skal kunne køres manuelt");
    QVERIFY2(wf.contains("proeve_skrynja_2"), "workflow skal køre værtssuiten før release");
    QVERIFY2(wf.contains("tjek_plan_manifest_spejl.py"), "workflow skal køre spejlkontrollen");
    QVERIFY2(wf.contains("contents: write"), "release-job skal have contents: write (GITHUB_TOKEN)");
    QVERIFY2(wf.contains("action-gh-release") || wf.contains("gh release create"), "workflow skal oprette GitHub Release");
    QVERIFY2(wf.contains("*.rpm"), "release skal vedhæfte rpm-filen");
    QVERIFY2(!wf.contains("git add"), "workflow må ikke commite rpm ind i depotet");
}

void TestByggestiOgLogfil::verificeringsskript_paa_enhed_findes_og_bestaaer_bash_n()
{
    QString sti = testRepoRoot() + "/værktøjer/verificer_paa_enhed.sh";
    QVERIFY2(QFileInfo::exists(sti), "værktøjer/verificer_paa_enhed.sh mangler");
    QString indhold = testReadSource("værktøjer/verificer_paa_enhed.sh");
    QVERIFY2(indhold.startsWith("#!"), "skript skal starte med shebang");
    QVERIFY2(indhold.contains("releases/latest"), "skript skal hente seneste GitHub Release");
    QVERIFY2(indhold.contains("aarch64.rpm"), "skript skal hente aarch64-rpm-asset");
    QVERIFY2(indhold.contains("rpm -qpl"), "skript skal køre rpm -qpl og tjekke indhold");
    QVERIFY2(indhold.contains("pkcon install-local"), "skript skal køre pkcon install-local");
    QVERIFY2(indhold.contains("/dev/tty"), "skript skal læse root-koden fra /dev/tty, så devel-su også virker når scriptet pipes (curl|bash) — koden må ikke logges");
    QVERIFY2(indhold.contains("--selvtest"), "skript skal køre appens --selvtest");
    QVERIFY2(indhold.contains("selvtest-verdict.json"), "skript skal udskrive verdict-JSON (og log-tail) efter --selvtest, så fejlsøgning på enheden ikke kræver manuel cat");
    QVERIFY2(indhold.contains("/usr/bin/harbour-skrynja-2"), "skript skal tjekke binær-stien");
    QVERIFY2(indhold.contains("icons/hicolor"), "skript skal tjekke ikon-stierne");
    QVERIFY2(indhold.contains("git push origin"), "skript skal udløse mål-byg og udgivelse via tag-push (git push origin vX.Y.Z) i samme kørsel");
    QVERIFY2(indhold.contains("releases/tag/"), "skript skal vente på release-siden for det nye tag før hentning, så intet manuelt trin står mellem byg og hentning");
    QVERIFY2(indhold.contains("releases/expanded_assets/"), "skript skal finde asset-navn via expanded_assets/<tag> først (fallback releases/tag/) — målt på Jolla Phone 2026: den fulde release-side gav intet asset-match på enheden");
    QVERIFY2(!indhold.contains("api.github.com"), "skript må ikke kalde api.github.com på enheden (403 målt 2026-11-25, ordre fra-13)");
    QVERIFY2(indhold.contains("url_effective"), "skript skal løse seneste tag via github.com-redirect (url_effective på releases/latest)");
    QVERIFY2(indhold.contains("releases/download/"), "skript skal hente asset via direkte download-URL (releases/download/<tag>/<fil>)");

    QProcess bash;
    bash.setWorkingDirectory(testRepoRoot());
    bash.start("bash", QStringList() << "-n" << sti);
    QVERIFY2(bash.waitForFinished(15000), "bash -n hang");
    QCOMPARE(bash.exitCode(), 0);

    QString simDir = QDir::tempPath() + "/skrynja-tst-simuleret-hent";
    QDir().mkpath(simDir + "/bin");
    QString logSti = simDir + "/kald.log";
    QString curlStub = R"STUB(#!/bin/bash
echo "$*" >> "$STUB_LOG"
url=""; fmt=""; out=""; prev=""
for a in "$@"; do
    case "$prev" in -o) out="$a";; -w) fmt="$a";; esac
    case "$a" in http*) url="$a";; esac
    prev="$a"
done
case "$url" in
    */releases/latest)
        if [ "$fmt" = "%{url_effective}" ]; then echo "https://github.com/storefyr/Skrynja_2/releases/tag/v1.0.2"; fi ;;
    */releases/expanded_assets/*)
        if [ "$fmt" = "%{http_code}" ]; then echo "200"; else
            echo '<a href="/storefyr/Skrynja_2/releases/download/v1.0.2/harbour-skrynja-2-1.0.2-1.aarch64.rpm">rpm</a>'
        fi ;;
    */releases/tag/*)
        if [ "$fmt" = "%{http_code}" ]; then echo "200"; else
            echo '<a href="/storefyr/Skrynja_2/releases/download/v1.0.2/harbour-skrynja-2-1.0.2-1.aarch64.rpm">rpm</a>'
        fi ;;
    */releases/download/*)
        if [ "$fmt" = "%{http_code}" ]; then echo "200"; else printf 'FAKE-RPM\n' > "$out"; fi ;;
    *) echo "stub: ukendt url: $url" >&2; exit 1 ;;
esac
exit 0
)STUB";
    QString rpmStub = R"STUB(#!/bin/bash
if [ "$1" = "-qpl" ]; then
cat <<'STIER'
/usr/bin/harbour-skrynja-2
/usr/share/applications/harbour-skrynja-2.desktop
/usr/share/harbour-skrynja-2/qml
/usr/share/harbour-skrynja-2/qml/harbour-skrynja-2.qml
/usr/share/harbour-skrynja-2/qml/pages
/usr/share/harbour-skrynja-2/qml/pages/DiffVisning.qml
/usr/share/harbour-skrynja-2/qml/pages/KonfigurationsSide.qml
/usr/share/harbour-skrynja-2/qml/pages/StartSide.qml
/usr/share/harbour-skrynja-2/qml/pages/TilladelsesDialog.qml
/usr/share/icons/hicolor/86x86/apps/harbour-skrynja-2.png
/usr/share/icons/hicolor/108x108/apps/harbour-skrynja-2.png
/usr/share/icons/hicolor/128x128/apps/harbour-skrynja-2.png
/usr/share/icons/hicolor/172x172/apps/harbour-skrynja-2.png
STIER
fi
exit 0
)STUB";
    QList<QPair<QString, QString> > stubs;
    stubs << qMakePair(QString("curl"), curlStub)
          << qMakePair(QString("rpm"), rpmStub)
          << qMakePair(QString("devel-su"), QString("#!/bin/bash\nexit 0\n"))
          << qMakePair(QString("harbour-skrynja-2"), QString("#!/bin/bash\necho '{\"resultat\":\"pass\"}'\nexit 0\n"));
    for (int i = 0; i < stubs.size(); ++i) {
        QFile f(simDir + "/bin/" + stubs.at(i).first);
        QVERIFY2(f.open(QIODevice::WriteOnly | QIODevice::Truncate), qPrintable("kan ikke skrive stub " + stubs.at(i).first));
        f.write(stubs.at(i).second.toUtf8());
        f.close();
        QVERIFY2(f.setPermissions(QFileDevice::ExeOwner | QFileDevice::ReadOwner | QFileDevice::WriteOwner), "kan ikke gøre stub eksekverbar");
    }
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", simDir + "/bin:" + env.value("PATH"));
    env.insert("STUB_LOG", logSti);
    QProcess sim;
    sim.setWorkingDirectory(simDir);
    sim.setProcessEnvironment(env);
    sim.start("bash", QStringList() << sti);
    QVERIFY2(sim.waitForFinished(30000), "simuleret kørsel hang");
    QCOMPARE(sim.exitCode(), 0);
    QFile logFil(logSti);
    QVERIFY2(logFil.open(QIODevice::ReadOnly), "kald-log mangler — skriptet kaldte ikke curl");
    QString kald = QString::fromUtf8(logFil.readAll());
    logFil.close();
    QVERIFY2(!kald.contains("api.github.com"), qPrintable("simuleret kørsel ramte api.github.com: " + kald));
    QVERIFY2(kald.contains("releases/latest"), qPrintable("seneste tag skal løses via releases/latest-redirect: " + kald));
    QVERIFY2(kald.contains("releases/download/v1.0.2/harbour-skrynja-2-1.0.2-1.aarch64.rpm"), qPrintable("asset skal hentes via direkte download-URL: " + kald));
    QVERIFY2(kald.contains("releases/expanded_assets/v1.0.2"), qPrintable("asset-navn skal slås op via expanded_assets-fragmentet først: " + kald));
}

#include "proeve_byggesti_og_logfil.moc"
