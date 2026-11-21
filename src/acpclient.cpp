#include "acpclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "acpframereader.h"
#include "agentproces.h"
#include "diffmodel.h"
#include "konfigurationsmodel.h"
#include "samtalemodel.h"
#include "sessionlager.h"
#include "tilladelsesmodel.h"
#include "vaerktoejskaldsmodel.h"

#include <QDir>

static const QString kSessionPrompt = QLatin1String("session/prompt");
static const QString kSessionCancel = QLatin1String("session/cancel");
static const QString kSetConfigOption = QLatin1String("session/set_config_option");

AcpClient::AcpClient(QObject* parent)
    : QObject(parent)
    , m_process(new AgentProcess(this))
    , m_reader(new AcpFrameReader(m_process->process(), this))
    , m_conversation(new ConversationModel(this))
    , m_store(new SessionStore)
    , m_toolCallModel(new ToolCallModel(this))
    , m_permissionModel(new PermissionModel(this))
    , m_diff(new DiffModel(this))
    , m_configModel(new ConfigurationModel(this))
    , m_nextId(1)
    , m_testMode(false)
    , m_hadRestoredHistory(false)
{
    connect(m_reader, SIGNAL(messageReady(QByteArray)), this, SLOT(receiveLine(QByteArray)));
    connect(m_process, SIGNAL(error(QString)), this, SLOT(receiveAgentError(QString)));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
    connect(m_toolCallModel, SIGNAL(changed()), this, SIGNAL(toolCallsChanged()));
    connect(m_conversation, SIGNAL(changed()), this, SIGNAL(conversationChanged()));
    connect(m_permissionModel, SIGNAL(changed()), this, SIGNAL(permissionChanged()));
    connect(m_diff, SIGNAL(changed()), this, SIGNAL(diffsChanged()));
    connect(m_diff, SIGNAL(changed()), this, SIGNAL(outputsChanged()));
    connect(m_configModel, SIGNAL(changed()), this, SIGNAL(configChanged()));
}

AcpClient::~AcpClient()
{
    m_process->stopAgent();
}

bool AcpClient::startAgent()
{
    if (m_testMode) {
        sendInitialize();
        return true;
    }
    if (!m_process->start()) {
        m_lastError = m_process->lastError();
        emit agentError(m_lastError);
        return false;
    }
    sendInitialize();
    return true;
}

void AcpClient::sendInitialize()
{
    QJsonObject params;
    params.insert("protocolVersion", 1);
    params.insert("clientCapabilities", QJsonObject());
    QJsonObject clientInfo;
    clientInfo.insert("name", "harbour-skrynja-2");
    clientInfo.insert("version", "1.0.0");
    params.insert("clientInfo", clientInfo);
    qint64 id = sendRequest("initialize", params);
    m_pendingRequests.insert(id, "initialize");
}

void AcpClient::startSession()
{
    if (m_store->hasSessionId())
        sendSessionLoad(m_store->sessionId());
    else
        sendSessionNew(QDir::homePath());
}

void AcpClient::sendSessionLoad(const QString& sessionId)
{
    QJsonObject params;
    params.insert("sessionId", sessionId);
    qint64 id = sendRequest("session/load", params);
    m_pendingRequests.insert(id, "session/load");
}

void AcpClient::sendSessionNew(const QString& cwd)
{
    QJsonObject params;
    params.insert("cwd", cwd);
    qint64 id = sendRequest("session/new", params);
    m_pendingRequests.insert(id, "session/new");
}

void AcpClient::sendSessionPrompt(const QString& prompt)
{
    m_conversation->startRound();
    QJsonObject blok;
    blok.insert("type", "text");
    blok.insert("text", prompt);
    QJsonArray blokke;
    blokke.append(blok);
    QJsonObject params;
    params.insert("sessionId", m_sessionId);
    params.insert("prompt", blokke);
    qint64 id = sendRequest(kSessionPrompt, params);
    m_pendingRequests.insert(id, kSessionPrompt);
}

ConversationModel* AcpClient::conversationModel() const
{
    return m_conversation;
}

ToolCallModel* AcpClient::toolCallModel() const
{
    return m_toolCallModel;
}

QVariantList AcpClient::toolCalls() const
{
    return m_toolCallModel->toolCalls();
}

QString AcpClient::conversationText() const
{
    return m_conversation->lastText();
}

int AcpClient::conversationRound() const
{
    return m_conversation->round();
}

QString AcpClient::conversationStatus() const
{
    return m_conversation->status();
}

void AcpClient::acknowledgeToolCall(const QString& toolCallId)
{
    m_toolCallModel->acknowledge(toolCallId);
}

bool AcpClient::answerPermission(const QString& optionId)
{
    if (!m_permissionModel->isPending())
        return false;
    QJsonObject outcome;
    outcome.insert("outcome", "selected");
    outcome.insert("optionId", optionId);
    QJsonObject result;
    result.insert("outcome", outcome);
    sendMessage(AcpMessage::response(m_permissionModel->requestId(), result));
    m_permissionModel->setAnswered();
    return true;
}

bool AcpClient::declinePermission()
{
    if (!m_permissionModel->isPending())
        return false;
    QJsonObject outcome;
    outcome.insert("outcome", "cancelled");
    QJsonObject result;
    result.insert("outcome", outcome);
    sendMessage(AcpMessage::response(m_permissionModel->requestId(), result));
    m_permissionModel->setAnswered();
    return true;
}

void AcpClient::sessionCancel()
{
    QJsonObject params;
    params.insert("sessionId", m_sessionId);
    sendNotification(kSessionCancel, params);
}

void AcpClient::stopAgent()
{
    m_process->stopAgent();
}

PermissionModel* AcpClient::permissionModel() const
{
    return m_permissionModel;
}

QVariantMap AcpClient::pendingPermission() const
{
    return m_permissionModel->pendingPermission();
}

int AcpClient::permissionCount() const
{
    return m_permissionModel->requests();
}

DiffModel* AcpClient::diffModel() const
{
    return m_diff;
}

QVariantList AcpClient::diffs() const
{
    return m_diff->diffs();
}

QVariantList AcpClient::outputs() const
{
    return m_diff->outputs();
}

ConfigurationModel* AcpClient::configurationModel() const
{
    return m_configModel;
}

void AcpClient::setConfigOption(const QString& configId, const QVariant& value)
{
    QJsonObject params;
    params.insert("sessionId", m_sessionId);
    params.insert("configId", configId);
    params.insert("value", QJsonValue::fromVariant(value));
    qint64 id = sendRequest(kSetConfigOption, params);
    m_pendingRequests.insert(id, kSetConfigOption);
}

QString AcpClient::agentPath() const
{
    return m_process->agentPath();
}

void AcpClient::setAgentsti(const QString& path)
{
    m_process->setAgentPath(path);
    emit agentPathChanged();
}

QString AcpClient::sessionId() const
{
    return m_sessionId;
}

QString AcpClient::stopReason() const
{
    return m_stopReason;
}

bool AcpClient::hadRestoredHistory() const
{
    return m_hadRestoredHistory;
}

QString AcpClient::lastError() const
{
    return m_lastError;
}

AgentProcess* AcpClient::agentProcess() const
{
    return m_process;
}

QVariantList AcpClient::configOptions() const
{
    return m_configOptions;
}

void AcpClient::setTestMode(bool tilstand)
{
    m_testMode = tilstand;
}

bool AcpClient::testMode() const
{
    return m_testMode;
}

void AcpClient::feedBytesForTest(const QByteArray& data)
{
    m_reader->feedBytesForTest(data);
}

void AcpClient::simulateAgentDeathForTest(int kode)
{
    processFinished(kode);
}

QList<QByteArray> AcpClient::sentForTest() const
{
    return m_sendte;
}

QByteArray AcpClient::lastSentForTest() const
{
    if (m_sendte.isEmpty())
        return QByteArray();
    return m_sendte.last();
}

void AcpClient::resetForTest()
{
    m_sendte.clear();
    m_pendingRequests.clear();
    m_configOptions.clear();
    m_lastError.clear();
    m_sessionId.clear();
    m_stopReason.clear();
}

qint64 AcpClient::nextId()
{
    return m_nextId++;
}

qint64 AcpClient::sendRequest(const QString& method, const QJsonObject& params)
{
    qint64 id = nextId();
    sendMessage(AcpMessage::request(id, method, params));
    return id;
}

void AcpClient::sendNotification(const QString& method, const QJsonObject& params)
{
    sendMessage(AcpMessage::notification(method, params));
}

void AcpClient::sendMessage(const AcpMessage& message)
{
    write(message.tilJson() + "\n");
}

void AcpClient::write(const QByteArray& line)
{
    if (m_testMode) {
        m_sendte.append(line);
    } else if (m_process->process()->isOpen()) {
        m_process->process()->write(line);
    }
    emit messageSent(line);
}

void AcpClient::receiveLine(const QByteArray& line)
{
    bool ok = false;
    AcpMessage message = AcpMessage::fraJson(line, &ok);
    if (ok)
        handleMessage(message);
}

void AcpClient::handleMessage(const AcpMessage& message)
{
    if (message.type() == AcpMessage::Response && message.hasId()) {
        QString method = m_pendingRequests.take(message.id());
        if (method == "session/load" && message.isError()) {
            m_hadRestoredHistory = false;
            emit historyChanged();
            sendSessionNew(QDir::homePath());
            return;
        }
        if (message.isError()) {
            m_lastError = message.error().value("message").toString();
            if (method == "initialize")
                emit agentError(m_lastError);
            return;
        }
        QJsonObject result = message.result().toObject();
        if (method == "initialize") {
            QJsonArray options = result.value("configOptions").toArray();
            m_configOptions.clear();
            for (int i = 0; i < options.size(); ++i)
                m_configOptions.append(options.at(i).toObject().toVariantMap());
            m_configModel->setConfigOptions(m_configOptions);
            emit initializeResponse(result);
            emit agentReady();
        } else if (method == "session/load") {
            m_sessionId = result.value("sessionId").toString();
            m_hadRestoredHistory = true;
            m_store->saveSessionId(m_sessionId);
            emit sessionCreated(m_sessionId);
            emit historyChanged();
        } else if (method == "session/new") {
            m_sessionId = result.value("sessionId").toString();
            m_hadRestoredHistory = false;
            m_store->saveSessionId(m_sessionId);
            emit sessionCreated(m_sessionId);
            emit historyChanged();
        } else if (method == kSessionPrompt) {
            m_stopReason = result.value("stopReason").toString();
        }
    } else if (message.type() == AcpMessage::Notification) {
        handleNotification(message);
    } else if (message.type() == AcpMessage::Request && message.hasId()) {
        if (message.method() == "session/request_permission") {
            m_permissionModel->setRequest(message.id(), message.params());
            emit permissionRequested();
        } else {
            sendMessage(AcpMessage::errorResponse(message.id(), -32601, "ukendt metode"));
        }
    }
}

void AcpClient::handleNotification(const AcpMessage& message)
{
    if (message.method() == "session/update")
        handleSessionUpdate(message.params());
    else if (message.method() == "end_turn") {
        m_conversation->endRound();
        emit roundFinished();
    }
}

void AcpClient::handleContentBlocks(const QJsonObject& update)
{
    QJsonArray content = update.value("content").toArray();
    for (int i = 0; i < content.size(); ++i) {
        QJsonObject blok = content.at(i).toObject();
        QString type = blok.value("type").toString();
        if (type == "diff")
            m_diff->addDiff(blok.value("path").toString(), blok.value("diff").toString());
        else if (type == "output")
            m_diff->addOutput(blok.value("text").toString());
    }
}

void AcpClient::handleSessionUpdate(const QJsonObject& params)
{
    QJsonObject update = params.value("update").toObject();
    QString art = update.value("sessionUpdate").toString();
    if (art == "agent_message_chunk") {
        QJsonObject content = update.value("content").toObject();
        m_conversation->addChunk(content.value("text").toString());
    } else if (art == "tool_call") {
        m_toolCallModel->addToolCall(update.value("toolCallId").toString(),
                                      update.value("title").toString(),
                                      update.value("status").toString());
        handleContentBlocks(update);
    } else if (art == "tool_call_update") {
        m_toolCallModel->updateToolCall(update.value("toolCallId").toString(),
                                         update.value("status").toString(),
                                         update.value("title").toString());
        handleContentBlocks(update);
    } else if (art == "turn_end") {
        m_conversation->endRound();
        emit roundFinished();
    }
}

void AcpClient::receiveAgentError(const QString& error)
{
    m_lastError = error;
    emit agentError(error);
}

void AcpClient::processFinished(int exitKode)
{
    emit agentDied(exitKode);
}
