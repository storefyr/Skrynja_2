#include "acpsession.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

#include "acpprotokol.h"
#include "sessionstore.h"

static const QString kSessionCancel = QLatin1String("session/cancel");

AcpSession::AcpSession(AcpProtokol* protokol, QObject* parent)
    : QObject(parent)
    , m_protokol(protokol)
    , m_store(new SessionStore)
    , m_hadRestoredHistory(false)
{
    connect(m_protokol, SIGNAL(sessionLoadFailed()), this, SLOT(onSessionLoadFailed()));
    connect(m_protokol, SIGNAL(sessionEstablished(QString,bool)), this, SLOT(onSessionEstablished(QString,bool)));
    connect(m_protokol, SIGNAL(promptResponded(QString)), this, SLOT(onPromptResponded(QString)));
}

QString AcpSession::sessionId() const
{
    return m_sessionId;
}

bool AcpSession::hadRestoredHistory() const
{
    return m_hadRestoredHistory;
}

QString AcpSession::stopReason() const
{
    return m_stopReason;
}

void AcpSession::initialize()
{
    QJsonObject params;
    params.insert("protocolVersion", 1);
    params.insert("clientCapabilities", QJsonObject());
    QJsonObject clientInfo;
    clientInfo.insert("name", "harbour-skrynja-2");
    clientInfo.insert("version", "1.0.0");
    params.insert("clientInfo", clientInfo);
    sendRequest(AcpProtokol::kInitialize, params);
}

void AcpSession::startSession()
{
    if (m_store->hasSessionId())
        sendSessionLoad(m_store->sessionId());
    else
        sendSessionNew(QDir::homePath());
}

void AcpSession::sendSessionNew(const QString& cwd)
{
    QJsonObject params;
    params.insert("cwd", cwd);
    sendRequest(AcpProtokol::kSessionNew, params);
}

void AcpSession::sendSessionLoad(const QString& sessionId)
{
    QJsonObject params;
    params.insert("sessionId", sessionId);
    sendRequest(AcpProtokol::kSessionLoad, params);
}

void AcpSession::sendSessionPrompt(const QString& prompt)
{
    emit promptStarted();
    QJsonObject blok;
    blok.insert("type", "text");
    blok.insert("text", prompt);
    QJsonArray blokke;
    blokke.append(blok);
    QJsonObject params;
    params.insert("sessionId", m_sessionId);
    params.insert("prompt", blokke);
    sendRequest(AcpProtokol::kSessionPrompt, params);
}

void AcpSession::sessionCancel()
{
    QJsonObject params;
    params.insert("sessionId", m_sessionId);
    sendNotification(kSessionCancel, params);
}

qint64 AcpSession::sendRequest(const QString& method, const QJsonObject& params)
{
    qint64 id = m_protokol->registerRequest(method);
    emit messageOut(AcpMessage::request(id, method, params));
    return id;
}

void AcpSession::sendNotification(const QString& method, const QJsonObject& params)
{
    emit messageOut(AcpMessage::notification(method, params));
}

void AcpSession::onSessionLoadFailed()
{
    m_hadRestoredHistory = false;
    emit historyChanged();
    sendSessionNew(QDir::homePath());
}

void AcpSession::onSessionEstablished(const QString& sessionId, bool restored)
{
    m_sessionId = sessionId;
    m_hadRestoredHistory = restored;
    m_store->saveSessionId(m_sessionId);
    emit sessionCreated(m_sessionId);
    emit historyChanged();
}

void AcpSession::onPromptResponded(const QString& stopReason)
{
    m_stopReason = stopReason;
}
