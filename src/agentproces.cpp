#include "agentproces.h"

#include <QFileInfo>
#include <QProcess>
#include <QSettings>

AgentProces::AgentProces(QObject* parent)
    : QObject(parent)
    , m_proces(new QProcess(this))
{
    connect(m_proces, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(procesFejl(QProcess::ProcessError)));
    connect(m_proces, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(procesFaerdig(int,QProcess::ExitStatus)));
}

QString AgentProces::agentsti() const
{
    QSettings indstillinger;
    return indstillinger.value("agent/sti", "/home/defaultuser/.local/bin/vibe-acp").toString();
}

void AgentProces::sætAgentsti(const QString& sti)
{
    QSettings indstillinger;
    indstillinger.setValue("agent/sti", sti);
    indstillinger.sync();
}

QString AgentProces::arbejdsmappe() const
{
    return m_arbejdsmappe;
}

void AgentProces::sætArbejdsmappe(const QString& mappe)
{
    m_arbejdsmappe = mappe;
}

bool AgentProces::start()
{
    m_lastError.clear();
    QString program = m_programForTest.isEmpty() ? agentsti() : m_programForTest;
    if (!QFileInfo::exists(program)) {
        m_lastError = "agent ikke fundet: " + program;
        emit fejl(m_lastError);
        return false;
    }
    if (m_proces->state() != QProcess::NotRunning)
        return true;
    m_proces->setProgram(program);
    m_proces->setArguments(m_argumenterForTest);
    if (!m_arbejdsmappe.isEmpty())
        m_proces->setWorkingDirectory(m_arbejdsmappe);
    m_proces->start();
    return true;
}

void AgentProces::stopAgent()
{
    if (m_proces->state() == QProcess::Running) {
        m_proces->terminate();
        if (!m_proces->waitForFinished(2000))
            m_proces->kill();
        m_proces->waitForFinished(1000);
    }
}

bool AgentProces::kører() const
{
    return m_proces->state() == QProcess::Running;
}

QString AgentProces::lastError() const
{
    return m_lastError;
}

QProcess* AgentProces::proces() const
{
    return m_proces;
}

void AgentProces::sætProgramOgArgumenterForTest(const QString& program, const QStringList& argumenter)
{
    m_programForTest = program;
    m_argumenterForTest = argumenter;
}

void AgentProces::procesFejl(QProcess::ProcessError fejlkode)
{
    Q_UNUSED(fejlkode);
    if (m_proces->error() == QProcess::FailedToStart)
        m_lastError = "agent kunne ikke startes: " + m_proces->program();
    else
        m_lastError = "agentproces fejlede";
    emit fejl(m_lastError);
}

void AgentProces::procesFaerdig(int exitKode, QProcess::ExitStatus status)
{
    Q_UNUSED(status);
    emit faerdig(exitKode);
}
