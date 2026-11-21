#ifndef AGENTPROCESS_H
#define AGENTPROCESS_H

#include <QProcess>
#include <QObject>
#include <QString>
#include <QStringList>

class AgentProcess : public QObject
{
    Q_OBJECT
public:
    explicit AgentProcess(QObject* parent = 0);
    AgentProcess(const QString& program, const QStringList& args, QObject* parent = 0);

    QString agentPath() const;
    void setAgentPath(const QString& path);

    bool start();
    bool stopAgent();
    bool isRunning() const;
    QString lastError() const;
    QProcess* process() const;

signals:
    void error(const QString& message);
    void finished(int exitKode);

private slots:
    void processError(QProcess::ProcessError errorCode);
    void processFinished(int exitKode, QProcess::ExitStatus status);

private:
    QProcess* m_process;
    QString m_lastError;
    QString m_program;
    QStringList m_args;
};

#endif
