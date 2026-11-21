#ifndef ACPCLIENT_H
#define ACPCLIENT_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QVariantList>

#include "acpmessage.h"

class QIODevice;
class AcpFrameReader;
class AcpProtokol;
class AcpSession;
class AgentProcess;
class DiffModel;
class ConfigurationModel;
class ConversationModel;
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

    virtual bool startAgent();
    void sendInitialize();
    QString lastError() const;
    AgentProcess* agentProcess() const;
    QVariantList configOptions() const;

    void setConversationModel(ConversationModel* model);
    void setToolCallModel(ToolCallModel* model);
    void setPermissionModel(PermissionModel* model);
    void setDiffModel(DiffModel* model);
    void setConfigurationModel(ConfigurationModel* model);

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
    AcpClient(AcpFrameReader* reader, QIODevice* udEnhed, QObject* parent = 0);

    qint64 sendRequest(const QString& method, const QJsonObject& params);
    void sendMessage(const AcpMessage& message);
    virtual void write(const QByteArray& line);

    AgentProcess* m_process;
    AcpFrameReader* m_reader;
    QIODevice* m_udEnhed;
    AcpProtokol* m_protokol;
    AcpSession* m_session;
    ConversationModel* m_conversation;
    ToolCallModel* m_toolCallModel;
    PermissionModel* m_permissionModel;
    DiffModel* m_diff;
    ConfigurationModel* m_configModel;
    QString m_lastError;
    QVariantList m_configOptions;

protected slots:
    void receiveLine(const QByteArray& line);
    void processFinished(int exitKode);

private slots:
    void receiveAgentError(const QString& error);
    void onMessageOut(const AcpMessage& message);
    void onResponseError(const QString& method, const QString& message);
    void onInitializeResponded(const QJsonObject& result);
    void onPromptStarted();
    void onPermissionRequest(qint64 requestId, const QJsonObject& params);
    void onAgentMessageChunk(const QString& text);
    void onToolCallAdded(const QString& toolCallId, const QString& title, const QString& status);
    void onToolCallUpdated(const QString& toolCallId, const QString& status, const QString& title);
    void onDiffContent(const QString& path, const QString& diff);
    void onOutputContent(const QString& text);
    void onRoundEnded();
    void onLineDropped(const QByteArray& line, const QString& reason);
    void onHaleReady(const QByteArray& hale);
};

#endif
