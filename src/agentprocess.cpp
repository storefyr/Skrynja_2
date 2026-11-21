#include "agentprocess.h"

#include <QFileInfo>
#include <QProcess>
#include <QSettings>

AgentProcess::AgentProcess(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(processFinished(int,QProcess::ExitStatus)));
}

AgentProcess::AgentProcess(const QString& program, const QStringList& args, QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_program(program)
    , m_args(args)
{
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(processFinished(int,QProcess::ExitStatus)));
}

QString AgentProcess::agentPath() const
{
    QSettings settings;
    return settings.value("agent/sti", "/home/defaultuser/.local/bin/vibe-acp").toString();
}

void AgentProcess::setAgentPath(const QString& path)
{
    QSettings settings;
    settings.setValue("agent/sti", path);
    settings.sync();
}

bool AgentProcess::start()
{
    m_lastError.clear();
    QString program = m_program.isEmpty() ? agentPath() : m_program;
    if (!QFileInfo::exists(program)) {
        m_lastError = "agent ikke fundet: " + program;
        emit error(m_lastError);
        return false;
    }
    if (m_process->state() != QProcess::NotRunning)
        return true;
    m_process->setProgram(program);
    m_process->setArguments(m_args);
    m_process->start();
    if (!m_process->waitForStarted(3000)) {
        m_lastError = "agent kunne ikke startes: " + program;
        emit error(m_lastError);
        return false;
    }
    return true;
}

bool AgentProcess::stopAgent()
{
    if (m_process->state() != QProcess::Running)
        return true;
    m_process->terminate();
    if (!m_process->waitForFinished(2000)) {
        m_process->kill();
        if (!m_process->waitForFinished(2000)) {
            m_lastError = "agent overlevede kill: " + m_process->program();
            emit error(m_lastError);
            return false;
        }
    }
    return true;
}

bool AgentProcess::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

QString AgentProcess::lastError() const
{
    return m_lastError;
}

QProcess* AgentProcess::process() const
{
    return m_process;
}

void AgentProcess::processError(QProcess::ProcessError errorCode)
{
    Q_UNUSED(errorCode);
    if (m_process->error() == QProcess::FailedToStart)
        m_lastError = "agent kunne ikke startes: " + m_process->program();
    else
        m_lastError = "agentproces fejlede";
    emit error(m_lastError);
}

void AgentProcess::processFinished(int exitKode, QProcess::ExitStatus status)
{
    Q_UNUSED(status);
    emit finished(exitKode);
}
