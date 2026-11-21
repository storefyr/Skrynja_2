#ifndef ACPCLIENT_H
#define ACPCLIENT_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QVariantList>

#include "acpbesked.h"

class AcpFrameReader;
class AgentProcess;
class DiffModel;
class ConfigurationModel;
class ConversationModel;
class SessionStore;
class PermissionModel;
class ToolCallModel;

class AcpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hadRestoredHistory READ hadRestoredHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList toolCalls READ toolCalls NOTIFY toolCallsChanged)
    Q_PROPERTY(QString conversationText READ conversationText NOTIFY conversationChanged)
    Q_PROPERTY(int conversationRound READ conversationRound NOTIFY conversationChanged)
    Q_PROPERTY(QString conversationStatus READ conversationStatus NOTIFY conversationChanged)
    Q_PROPERTY(QVariantMap pendingPermission READ pendingPermission NOTIFY permissionChanged)
    Q_PROPERTY(int permissionCount READ permissionCount NOTIFY permissionChanged)
    Q_PROPERTY(QVariantList diffs READ diffs NOTIFY diffsChanged)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY outputsChanged)
    Q_PROPERTY(QVariantList configOptions READ configOptions NOTIFY configChanged)
    Q_PROPERTY(QString agentPath READ agentPath NOTIFY agentPathChanged)
public:
    explicit AcpClient(QObject* parent = 0);
    ~AcpClient();

    bool startAgent();
    void sendInitialize();
    QString lastError() const;
    AgentProcess* agentProcess() const;
    QVariantList configOptions() const;

    Q_INVOKABLE void startSession();
    Q_INVOKABLE void sendSessionNew(const QString& cwd);
    Q_INVOKABLE void sendSessionPrompt(const QString& prompt);
    ConversationModel* conversationModel() const;
    ToolCallModel* toolCallModel() const;
    QVariantList toolCalls() const;
    QString conversationText() const;
    int conversationRound() const;
    QString conversationStatus() const;
    Q_INVOKABLE void acknowledgeToolCall(const QString& toolCallId);
    Q_INVOKABLE bool answerPermission(const QString& optionId);
    Q_INVOKABLE bool declinePermission();
    Q_INVOKABLE void sessionCancel();
    Q_INVOKABLE void stopAgent();
    PermissionModel* permissionModel() const;
    QVariantMap pendingPermission() const;
    int permissionCount() const;
    DiffModel* diffModel() const;
    QVariantList diffs() const;
    QVariantList outputs() const;
    ConfigurationModel* configurationModel() const;
    Q_INVOKABLE void setConfigOption(const QString& configId, const QVariant& value);
    QString agentPath() const;
    Q_INVOKABLE void setAgentsti(const QString& path);
    QString sessionId() const;
    QString stopReason() const;
    bool hadRestoredHistory() const;

    void setTestMode(bool tilstand);
    bool testMode() const;
    void feedBytesForTest(const QByteArray& data);
    void simulateAgentDeathForTest(int kode);
    QList<QByteArray> sentForTest() const;
    QByteArray lastSentForTest() const;
    void resetForTest();

signals:
    void messageSent(const QByteArray& line);
    void agentReady();
    void agentError(const QString& error);
    void initializeResponse(const QJsonObject& result);
    void sessionCreated(const QString& sessionId);
    void roundFinished();
    void historyChanged();
    void toolCallsChanged();
    void conversationChanged();
    void permissionRequested();
    void permissionChanged();
    void agentDied(int kode);
    void diffsChanged();
    void outputsChanged();
    void configChanged();
    void agentPathChanged();

protected:
    qint64 nextId();
    qint64 sendRequest(const QString& method, const QJsonObject& params);
    void sendSessionLoad(const QString& sessionId);
    void sendNotification(const QString& method, const QJsonObject& params);
    void sendMessage(const AcpMessage& message);
    void write(const QByteArray& line);
    virtual void handleMessage(const AcpMessage& message);
    void handleNotification(const AcpMessage& message);
    void handleSessionUpdate(const QJsonObject& params);
    void handleContentBlocks(const QJsonObject& update);

    AgentProcess* m_process;
    AcpFrameReader* m_reader;
    ConversationModel* m_conversation;
    SessionStore* m_store;
    ToolCallModel* m_toolCallModel;
    PermissionModel* m_permissionModel;
    DiffModel* m_diff;
    ConfigurationModel* m_configModel;
    qint64 m_nextId;
    bool m_testMode;
    QList<QByteArray> m_sendte;
    QHash<qint64, QString> m_pendingRequests;
    QString m_lastError;
    QString m_sessionId;
    QString m_stopReason;
    bool m_hadRestoredHistory;
    QVariantList m_configOptions;

private slots:
    void receiveLine(const QByteArray& line);
    void receiveAgentError(const QString& error);
    void processFinished(int exitKode);
};

#endif
