#ifndef SELFTEST_H
#define SELFTEST_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class AcpClient;

class SelfTest : public QObject
{
    Q_OBJECT
public:
    explicit SelfTest(AcpClient* client, QObject* parent = 0);

    void start(const QString& prompt, const QString& verdictPath);
    bool verdictWritten() const;
    QJsonObject verdict() const;
    QString verdictPath() const;

signals:
    void finished(int exitKode);

private slots:
    void onAgentReady();
    void onSessionCreated(const QString& sessionId);
    void onPermissionRequested();
    void onRoundFinished();
    void onAgentError(const QString& error);
    void onAgentDied(int kode);
    void onMessageSent(const QByteArray& line);
    void onDiffsChanged();

private:
    void writeVerdict(bool pass, const QString& reason);
    void finish(bool pass, const QString& reason);

    AcpClient* m_client;
    QString m_prompt;
    QString m_verdictPath;
    bool m_verdictWritten;
    bool m_promptSent;
    bool m_permissionAnswered;
    bool m_fileWritten;
};

#endif
