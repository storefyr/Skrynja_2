#ifndef ACPSESSION_H
#define ACPSESSION_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "acpbesked.h"

class AcpProtokol;
class SessionStore;

class AcpSession : public QObject
{
    Q_OBJECT
public:
    explicit AcpSession(AcpProtokol* protokol, QObject* parent = 0);

    QString sessionId() const;
    bool hadRestoredHistory() const;
    QString stopReason() const;

    void initialize();
    void startSession();
    void sendSessionNew(const QString& cwd);
    void sendSessionLoad(const QString& sessionId);
    void sendSessionPrompt(const QString& prompt);
    void sessionCancel();

    void resetForTest();

signals:
    void messageOut(const AcpMessage& message);
    void sessionCreated(const QString& sessionId);
    void historyChanged();
    void promptStarted();

private slots:
    void onSessionLoadFailed();
    void onSessionEstablished(const QString& sessionId, bool restored);
    void onPromptResponded(const QString& stopReason);

private:
    qint64 sendRequest(const QString& method, const QJsonObject& params);
    void sendNotification(const QString& method, const QJsonObject& params);

    AcpProtokol* m_protokol;
    SessionStore* m_store;
    QString m_sessionId;
    QString m_stopReason;
    bool m_hadRestoredHistory;
};

#endif
