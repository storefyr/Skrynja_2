#include "acpprotokol.h"

#include <QJsonArray>
#include <QJsonObject>

const QString AcpProtokol::kInitialize = QLatin1String("initialize");
const QString AcpProtokol::kSessionLoad = QLatin1String("session/load");
const QString AcpProtokol::kSessionNew = QLatin1String("session/new");
const QString AcpProtokol::kSessionPrompt = QLatin1String("session/prompt");
const QString AcpProtokol::kSessionUpdate = QLatin1String("session/update");
const QString AcpProtokol::kEndTurn = QLatin1String("end_turn");
const QString AcpProtokol::kRequestPermission = QLatin1String("session/request_permission");

AcpProtokol::AcpProtokol(QObject* parent)
    : QObject(parent)
    , m_nextId(1)
{
}

qint64 AcpProtokol::nextId()
{
    return m_nextId++;
}

qint64 AcpProtokol::registerRequest(const QString& method)
{
    qint64 id = nextId();
    m_pendingRequests.insert(id, method);
    return id;
}

QString AcpProtokol::takeRequestMethod(qint64 id)
{
    return m_pendingRequests.take(id);
}

bool AcpProtokol::hasPendingRequest(qint64 id) const
{
    return m_pendingRequests.contains(id);
}

int AcpProtokol::pendingRequestCount() const
{
    return m_pendingRequests.size();
}

void AcpProtokol::clearPendingRequests()
{
    m_pendingRequests.clear();
}

void AcpProtokol::handleMessage(const AcpMessage& message)
{
    if (message.type() == AcpMessage::Response && message.hasId()) {
        QString method = takeRequestMethod(message.id());
        if (method == kSessionLoad && message.isError()) {
            emit sessionLoadFailed();
            return;
        }
        if (message.isError()) {
            emit responseError(method, message.error().value("message").toString());
            return;
        }
        if (method.isEmpty()) {
            emit responseError(method, QString("ukendt svar-id %1").arg(message.id()));
            return;
        }
        QJsonObject result = message.result().toObject();
        if (method == kInitialize) {
            emit initializeResponded(result);
        } else if (method == kSessionLoad || method == kSessionNew) {
            emit sessionEstablished(result.value("sessionId").toString(), method == kSessionLoad);
        } else if (method == kSessionPrompt) {
            emit promptResponded(result.value("stopReason").toString());
        }
    } else if (message.type() == AcpMessage::Notification) {
        handleNotification(message);
    } else if (message.type() == AcpMessage::Request && message.hasId()) {
        if (message.method() == kRequestPermission) {
            emit permissionRequestReceived(message.id(), message.params());
        } else {
            emit messageOut(AcpMessage::errorResponse(message.id(), -32601, "ukendt metode"));
        }
    }
}

void AcpProtokol::handleNotification(const AcpMessage& message)
{
    if (message.method() == kSessionUpdate)
        handleSessionUpdate(message.params());
    else if (message.method() == kEndTurn)
        emit roundEnded();
}

void AcpProtokol::handleSessionUpdate(const QJsonObject& params)
{
    QJsonObject update = params.value("update").toObject();
    QString art = update.value("sessionUpdate").toString();
    if (art == "agent_message_chunk") {
        QJsonObject content = update.value("content").toObject();
        emit agentMessageChunk(content.value("text").toString());
    } else if (art == "tool_call") {
        emit toolCallAdded(update.value("toolCallId").toString(),
                           update.value("title").toString(),
                           update.value("status").toString());
        handleContentBlocks(update);
    } else if (art == "tool_call_update") {
        emit toolCallUpdated(update.value("toolCallId").toString(),
                             update.value("status").toString(),
                             update.value("title").toString());
        handleContentBlocks(update);
    } else if (art == "turn_end") {
        emit roundEnded();
    }
}

void AcpProtokol::handleContentBlocks(const QJsonObject& update)
{
    QJsonArray content = update.value("content").toArray();
    for (int i = 0; i < content.size(); ++i) {
        QJsonObject blok = content.at(i).toObject();
        QString type = blok.value("type").toString();
        if (type == "diff")
            emit diffContent(blok.value("path").toString(), blok.value("diff").toString());
        else if (type == "output")
            emit outputContent(blok.value("text").toString());
    }
}
