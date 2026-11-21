#ifndef ACPPROTOKOL_H
#define ACPPROTOKOL_H

#include <QHash>
#include <QObject>
#include <QString>

#include "acpmessage.h"

class AcpProtokol : public QObject
{
    Q_OBJECT
public:
    static const QString kInitialize;
    static const QString kSessionLoad;
    static const QString kSessionNew;
    static const QString kSessionPrompt;
    static const QString kSessionCancel;
    static const QString kSetConfigOption;
    static const QString kSessionUpdate;
    static const QString kEndTurn;
    static const QString kRequestPermission;

    explicit AcpProtokol(QObject* parent = 0);

    qint64 nextId();
    qint64 registerRequest(const QString& method);
    QString takeRequestMethod(qint64 id);
    bool hasPendingRequest(qint64 id) const;
    int pendingRequestCount() const;
    void clearPendingRequests();

    void handleMessage(const AcpMessage& message);
    void handleNotification(const AcpMessage& message);
    void handleSessionUpdate(const QJsonObject& params);

signals:
    void messageOut(const AcpMessage& message);
    void responseError(const QString& method, const QString& message);
    void initializeResponded(const QJsonObject& result);
    void sessionLoadFailed();
    void sessionEstablished(const QString& sessionId, bool restored);
    void promptResponded(const QString& stopReason);
    void permissionRequestReceived(qint64 requestId, const QJsonObject& params);
    void agentMessageChunk(const QString& text);
    void toolCallAdded(const QString& toolCallId, const QString& title, const QString& status);
    void toolCallUpdated(const QString& toolCallId, const QString& status, const QString& title);
    void diffContent(const QString& path, const QString& diff);
    void outputContent(const QString& text);
    void roundEnded();

private:
    void handleContentBlocks(const QJsonObject& update);

    qint64 m_nextId;
    QHash<qint64, QString> m_pendingRequests;
};

#endif
