#ifndef AGENTPROCES_H
#define AGENTPROCES_H

#include <QProcess>
#include <QObject>
#include <QString>
#include <QStringList>

class AgentProces : public QObject
{
    Q_OBJECT
public:
    explicit AgentProces(QObject* parent = 0);

    QString agentsti() const;
    void sætAgentsti(const QString& sti);
    QString arbejdsmappe() const;
    void sætArbejdsmappe(const QString& mappe);

    bool start();
    void stopAgent();
    bool kører() const;
    QString lastError() const;
    QProcess* proces() const;

    void sætProgramOgArgumenterForTest(const QString& program, const QStringList& argumenter);

signals:
    void fejl(const QString& besked);
    void faerdig(int exitKode);

private slots:
    void procesFejl(QProcess::ProcessError fejlkode);
    void procesFaerdig(int exitKode, QProcess::ExitStatus status);

private:
    QProcess* m_proces;
    QString m_lastError;
    QString m_arbejdsmappe;
    QString m_programForTest;
    QStringList m_argumenterForTest;
};

#endif
