#include "acpclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "acpframereader.h"
#include "acpprotokol.h"
#include "acpsession.h"
#include "agentprocess.h"
#include "diffmodel.h"
#include "configurationmodel.h"
#include "conversationmodel.h"
#include "logfile.h"
#include "permissionmodel.h"
#include "toolcallmodel.h"

static const QString kSetConfigOption = QLatin1String("session/set_config_option");

AcpClient::AcpClient(QObject* parent)
    : QObject(parent)
    , m_process(new AgentProcess(this))
    , m_reader(new AcpFrameReader(m_process->process(), this))
    , m_protokol(new AcpProtokol(this))
    , m_session(new AcpSession(m_protokol, this))
    , m_conversation(0)
    , m_toolCallModel(0)
    , m_permissionModel(0)
    , m_diff(0)
    , m_configModel(0)
    , m_testMode(false)
{
    connect(m_reader, SIGNAL(messageReady(QByteArray)), this, SLOT(receiveLine(QByteArray)));
    connect(m_reader, SIGNAL(lineDropped(QByteArray,QString)), this, SLOT(onLineDropped(QByteArray,QString)));
    connect(m_reader, SIGNAL(haleReady(QByteArray)), this, SLOT(onHaleReady(QByteArray)));
    connect(m_process, SIGNAL(error(QString)), this, SLOT(receiveAgentError(QString)));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));
    connect(m_protokol, SIGNAL(messageOut(AcpMessage)), this, SLOT(onMessageOut(AcpMessage)));
    connect(m_protokol, SIGNAL(responseError(QString,QString)), this, SLOT(onResponseError(QString,QString)));
    connect(m_protokol, SIGNAL(initializeResponded(QJsonObject)), this, SLOT(onInitializeResponded(QJsonObject)));
    connect(m_protokol, SIGNAL(permissionRequestReceived(qint64,QJsonObject)), this, SLOT(onPermissionRequest(qint64,QJsonObject)));
    connect(m_session, SIGNAL(messageOut(AcpMessage)), this, SLOT(onMessageOut(AcpMessage)));
    connect(m_session, SIGNAL(sessionCreated(QString)), this, SIGNAL(sessionCreated(QString)));
    connect(m_session, SIGNAL(historyChanged()), this, SIGNAL(historyChanged()));
    connect(m_session, SIGNAL(promptStarted()), this, SLOT(onPromptStarted()));
    connect(m_protokol, SIGNAL(agentMessageChunk(QString)), this, SLOT(onAgentMessageChunk(QString)));
    connect(m_protokol, SIGNAL(toolCallAdded(QString,QString,QString)), this, SLOT(onToolCallAdded(QString,QString,QString)));
    connect(m_protokol, SIGNAL(toolCallUpdated(QString,QString,QString)), this, SLOT(onToolCallUpdated(QString,QString,QString)));
    connect(m_protokol, SIGNAL(diffContent(QString,QString)), this, SLOT(onDiffContent(QString,QString)));
    connect(m_protokol, SIGNAL(outputContent(QString)), this, SLOT(onOutputContent(QString)));
    connect(m_protokol, SIGNAL(roundEnded()), this, SLOT(onRoundEnded()));
}

AcpClient::~AcpClient()
{
    m_process->stopAgent();
}

void AcpClient::setConversationModel(ConversationModel* model)
{
    m_conversation = model;
    if (m_conversation)
        connect(m_conversation, SIGNAL(changed()), this, SIGNAL(conversationChanged()));
}

void AcpClient::setToolCallModel(ToolCallModel* model)
{
    m_toolCallModel = model;
    if (m_toolCallModel)
        connect(m_toolCallModel, SIGNAL(changed()), this, SIGNAL(toolCallsChanged()));
}

void AcpClient::setPermissionModel(PermissionModel* model)
{
    m_permissionModel = model;
    if (m_permissionModel)
        connect(m_permissionModel, SIGNAL(changed()), this, SIGNAL(permissionChanged()));
}

void AcpClient::setDiffModel(DiffModel* model)
{
    m_diff = model;
    if (m_diff) {
        connect(m_diff, SIGNAL(changed()), this, SIGNAL(diffsChanged()));
        connect(m_diff, SIGNAL(changed()), this, SIGNAL(outputsChanged()));
    }
}

void AcpClient::setConfigurationModel(ConfigurationModel* model)
{
    m_configModel = model;
    if (m_configModel)
        connect(m_configModel, SIGNAL(changed()), this, SIGNAL(configChanged()));
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
    m_session->initialize();
}

void AcpClient::startSession()
{
    m_session->startSession();
}

void AcpClient::sendSessionNew(const QString& cwd)
{
    m_session->sendSessionNew(cwd);
}

void AcpClient::sendSessionPrompt(const QString& prompt)
{
    m_session->sendSessionPrompt(prompt);
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
    return m_toolCallModel ? m_toolCallModel->toolCalls() : QVariantList();
}

QString AcpClient::conversationText() const
{
    return m_conversation ? m_conversation->lastText() : QString();
}

int AcpClient::conversationRound() const
{
    return m_conversation ? m_conversation->round() : 0;
}

QString AcpClient::conversationStatus() const
{
    return m_conversation ? m_conversation->status() : QString();
}

void AcpClient::acknowledgeToolCall(const QString& toolCallId)
{
    if (m_toolCallModel)
        m_toolCallModel->acknowledge(toolCallId);
}

bool AcpClient::answerPermission(const QString& optionId)
{
    if (!m_permissionModel || !m_permissionModel->isPending())
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
    if (!m_permissionModel || !m_permissionModel->isPending())
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
    m_session->sessionCancel();
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
    return m_permissionModel ? m_permissionModel->pendingPermission() : QVariantMap();
}

int AcpClient::permissionCount() const
{
    return m_permissionModel ? m_permissionModel->requests() : 0;
}

DiffModel* AcpClient::diffModel() const
{
    return m_diff;
}

QVariantList AcpClient::diffs() const
{
    return m_diff ? m_diff->diffs() : QVariantList();
}

QVariantList AcpClient::outputs() const
{
    return m_diff ? m_diff->outputs() : QVariantList();
}

ConfigurationModel* AcpClient::configurationModel() const
{
    return m_configModel;
}

void AcpClient::setConfigOption(const QString& configId, const QVariant& value)
{
    QJsonObject params;
    params.insert("sessionId", m_session->sessionId());
    params.insert("configId", configId);
    params.insert("value", QJsonValue::fromVariant(value));
    sendRequest(kSetConfigOption, params);
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
    return m_session->sessionId();
}

QString AcpClient::stopReason() const
{
    return m_session->stopReason();
}

bool AcpClient::hadRestoredHistory() const
{
    return m_session->hadRestoredHistory();
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

void AcpClient::eofForTest()
{
    m_reader->eofForTest();
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

qint64 AcpClient::sendRequest(const QString& method, const QJsonObject& params)
{
    qint64 id = m_protokol->registerRequest(method);
    sendMessage(AcpMessage::request(id, method, params));
    return id;
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
        m_protokol->handleMessage(message);
}

void AcpClient::onMessageOut(const AcpMessage& message)
{
    sendMessage(message);
}

void AcpClient::onResponseError(const QString& method, const QString& message)
{
    Q_UNUSED(method);
    m_lastError = message;
    emit agentError(m_lastError);
}

void AcpClient::onInitializeResponded(const QJsonObject& result)
{
    QJsonArray options = result.value("configOptions").toArray();
    m_configOptions.clear();
    for (int i = 0; i < options.size(); ++i)
        m_configOptions.append(options.at(i).toObject().toVariantMap());
    if (m_configModel)
        m_configModel->setConfigOptions(m_configOptions);
    emit initializeResponse(result);
    emit agentReady();
}

void AcpClient::onPromptStarted()
{
    if (m_conversation)
        m_conversation->startRound();
}

void AcpClient::onPermissionRequest(qint64 requestId, const QJsonObject& params)
{
    if (m_permissionModel)
        m_permissionModel->setRequest(requestId, params);
    emit permissionRequested();
}

void AcpClient::onAgentMessageChunk(const QString& text)
{
    if (m_conversation)
        m_conversation->addChunk(text);
}

void AcpClient::onToolCallAdded(const QString& toolCallId, const QString& title, const QString& status)
{
    if (m_toolCallModel)
        m_toolCallModel->addToolCall(toolCallId, title, status);
}

void AcpClient::onToolCallUpdated(const QString& toolCallId, const QString& status, const QString& title)
{
    if (m_toolCallModel)
        m_toolCallModel->updateToolCall(toolCallId, status, title);
}

void AcpClient::onDiffContent(const QString& path, const QString& diff)
{
    if (m_diff)
        m_diff->addDiff(path, diff);
}

void AcpClient::onOutputContent(const QString& text)
{
    if (m_diff)
        m_diff->addOutput(text);
}

void AcpClient::onRoundEnded()
{
    if (m_conversation)
        m_conversation->endRound();
    emit roundFinished();
}

void AcpClient::onLineDropped(const QByteArray& line, const QString& reason)
{
    LogFile::instance()->writeLine(QString("lineDropped (%1): %2 bytes").arg(reason).arg(line.size()));
}

void AcpClient::onHaleReady(const QByteArray& hale)
{
    LogFile::instance()->writeLine(QString("haleReady ved EOF: %1 bytes").arg(hale.size()));
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
