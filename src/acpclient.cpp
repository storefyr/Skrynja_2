#include "acpclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "acpframereader.h"
#include "agentproces.h"
#include "samtalemodel.h"
#include "sessionlager.h"
#include "tilladelsesmodel.h"
#include "vaerktoejskaldsmodel.h"

#include <QDir>

static const QString kSessionPrompt = QLatin1String("session/prompt");

AcpClient::AcpClient(QObject* parent)
    : QObject(parent)
    , m_proces(new AgentProces(this))
    , m_læser(new AcpFrameReader(m_proces->proces(), this))
    , m_samtale(new SamtaleModel(this))
    , m_lager(new SessionLager)
    , m_vaerktoejskald(new VaerktoejskaldsModel(this))
    , m_tilladelse(new Tilladelsesmodel(this))
    , m_næsteId(1)
    , m_testTilstand(false)
    , m_hadRestoredHistory(false)
{
    connect(m_læser, SIGNAL(beskedKlar(QByteArray)), this, SLOT(modtagLinje(QByteArray)));
    connect(m_proces, SIGNAL(fejl(QString)), this, SLOT(modtagAgentFejl(QString)));
    connect(m_vaerktoejskald, SIGNAL(changed()), this, SIGNAL(vaerktoejskaldAendret()));
    connect(m_samtale, SIGNAL(changed()), this, SIGNAL(samtaleAendret()));
    connect(m_tilladelse, SIGNAL(changed()), this, SIGNAL(tilladelseAendret()));
}

AcpClient::~AcpClient()
{
    m_proces->stopAgent();
}

bool AcpClient::startAgent()
{
    if (m_testTilstand) {
        sendInitialize();
        return true;
    }
    if (!m_proces->start()) {
        m_lastError = m_proces->lastError();
        emit agentFejl(m_lastError);
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
    m_åbneAnmodninger.insert(id, "initialize");
}

void AcpClient::startSession()
{
    if (m_lager->harSessionId())
        sendSessionLoad(m_lager->sessionId());
    else
        sendSessionNew(QDir::homePath());
}

void AcpClient::sendSessionLoad(const QString& sessionId)
{
    QJsonObject params;
    params.insert("sessionId", sessionId);
    qint64 id = sendRequest("session/load", params);
    m_åbneAnmodninger.insert(id, "session/load");
}

void AcpClient::sendSessionNew(const QString& cwd)
{
    QJsonObject params;
    params.insert("cwd", cwd);
    qint64 id = sendRequest("session/new", params);
    m_åbneAnmodninger.insert(id, "session/new");
}

void AcpClient::sendSessionPrompt(const QString& prompt)
{
    m_samtale->startRunde();
    QJsonObject blok;
    blok.insert("type", "text");
    blok.insert("text", prompt);
    QJsonArray blokke;
    blokke.append(blok);
    QJsonObject params;
    params.insert("sessionId", m_sessionId);
    params.insert("prompt", blokke);
    qint64 id = sendRequest(kSessionPrompt, params);
    m_åbneAnmodninger.insert(id, kSessionPrompt);
}

SamtaleModel* AcpClient::samtaleModel() const
{
    return m_samtale;
}

VaerktoejskaldsModel* AcpClient::vaerktoejskaldsModel() const
{
    return m_vaerktoejskald;
}

QVariantList AcpClient::toolkald() const
{
    return m_vaerktoejskald->toolCalls();
}

QString AcpClient::samtaleTekst() const
{
    return m_samtale->lastText();
}

int AcpClient::samtaleRunde() const
{
    return m_samtale->round();
}

QString AcpClient::samtaleStatus() const
{
    return m_samtale->status();
}

void AcpClient::kvitterVaerktoejskald(const QString& toolCallId)
{
    m_vaerktoejskald->acknowledge(toolCallId);
}

bool AcpClient::answerPermission(const QString& optionId)
{
    if (!m_tilladelse->afventende())
        return false;
    QJsonObject outcome;
    outcome.insert("outcome", "selected");
    outcome.insert("optionId", optionId);
    QJsonObject result;
    result.insert("outcome", outcome);
    sendBesked(AcpBesked::svar(m_tilladelse->anmodningsId(), result));
    m_tilladelse->setSvaret();
    return true;
}

bool AcpClient::afvisPermission()
{
    if (!m_tilladelse->afventende())
        return false;
    QJsonObject outcome;
    outcome.insert("outcome", "cancelled");
    QJsonObject result;
    result.insert("outcome", outcome);
    sendBesked(AcpBesked::svar(m_tilladelse->anmodningsId(), result));
    m_tilladelse->setSvaret();
    return true;
}

Tilladelsesmodel* AcpClient::tilladelsesmodel() const
{
    return m_tilladelse;
}

QVariantMap AcpClient::pendingPermission() const
{
    return m_tilladelse->pendingPermission();
}

int AcpClient::tilladelseAntal() const
{
    return m_tilladelse->anmodninger();
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

AgentProces* AcpClient::agentProces() const
{
    return m_proces;
}

QVariantList AcpClient::configOptions() const
{
    return m_configOptions;
}

void AcpClient::sætTestTilstand(bool tilstand)
{
    m_testTilstand = tilstand;
}

bool AcpClient::testTilstand() const
{
    return m_testTilstand;
}

void AcpClient::feedBytesForTest(const QByteArray& data)
{
    m_læser->feedBytesForTest(data);
}

QList<QByteArray> AcpClient::sendteForTest() const
{
    return m_sendte;
}

QByteArray AcpClient::sidsteSendtForTest() const
{
    if (m_sendte.isEmpty())
        return QByteArray();
    return m_sendte.last();
}

void AcpClient::nulstilForTest()
{
    m_sendte.clear();
    m_åbneAnmodninger.clear();
    m_configOptions.clear();
    m_lastError.clear();
    m_sessionId.clear();
    m_stopReason.clear();
}

qint64 AcpClient::næsteId()
{
    return m_næsteId++;
}

qint64 AcpClient::sendRequest(const QString& metode, const QJsonObject& params)
{
    qint64 id = næsteId();
    sendBesked(AcpBesked::anmodning(id, metode, params));
    return id;
}

void AcpClient::sendNotifikation(const QString& metode, const QJsonObject& params)
{
    sendBesked(AcpBesked::notifikation(metode, params));
}

void AcpClient::sendBesked(const AcpBesked& besked)
{
    skriv(besked.tilJson() + "\n");
}

void AcpClient::skriv(const QByteArray& linje)
{
    if (m_testTilstand) {
        m_sendte.append(linje);
        emit beskedSendt(linje);
    } else if (m_proces->proces()->isOpen()) {
        m_proces->proces()->write(linje);
    }
}

void AcpClient::modtagLinje(const QByteArray& linje)
{
    bool ok = false;
    AcpBesked besked = AcpBesked::fraJson(linje, &ok);
    if (ok)
        håndterBesked(besked);
}

void AcpClient::håndterBesked(const AcpBesked& besked)
{
    if (besked.type() == AcpBesked::Svar && besked.harId()) {
        QString metode = m_åbneAnmodninger.take(besked.id());
        if (metode == "session/load" && besked.erFejl()) {
            m_hadRestoredHistory = false;
            emit historieAendret();
            sendSessionNew(QDir::homePath());
            return;
        }
        if (besked.erFejl()) {
            m_lastError = besked.fejl().value("message").toString();
            if (metode == "initialize")
                emit agentFejl(m_lastError);
            return;
        }
        QJsonObject resultat = besked.resultat().toObject();
        if (metode == "initialize") {
            QJsonArray options = resultat.value("configOptions").toArray();
            m_configOptions.clear();
            for (int i = 0; i < options.size(); ++i)
                m_configOptions.append(options.at(i).toObject().toVariantMap());
            emit initializeSvar(resultat);
            emit agentKlar();
        } else if (metode == "session/load") {
            m_sessionId = resultat.value("sessionId").toString();
            m_hadRestoredHistory = true;
            m_lager->gemSessionId(m_sessionId);
            emit sessionOprettet(m_sessionId);
            emit historieAendret();
        } else if (metode == "session/new") {
            m_sessionId = resultat.value("sessionId").toString();
            m_hadRestoredHistory = false;
            m_lager->gemSessionId(m_sessionId);
            emit sessionOprettet(m_sessionId);
            emit historieAendret();
        } else if (metode == kSessionPrompt) {
            m_stopReason = resultat.value("stopReason").toString();
        }
    } else if (besked.type() == AcpBesked::Notifikation) {
        håndterNotifikation(besked);
    } else if (besked.type() == AcpBesked::Anmodning && besked.harId()) {
        if (besked.metode() == "session/request_permission") {
            m_tilladelse->setAnmodning(besked.id(), besked.params());
            emit permissionAnmodet();
        } else {
            sendBesked(AcpBesked::fejlSvar(besked.id(), -32601, "ukendt metode"));
        }
    }
}

void AcpClient::håndterNotifikation(const AcpBesked& besked)
{
    if (besked.metode() == "session/update")
        håndterSessionUpdate(besked.params());
    else if (besked.metode() == "end_turn") {
        m_samtale->endRunde();
        emit rundeAfsluttet();
    }
}

void AcpClient::håndterSessionUpdate(const QJsonObject& params)
{
    QJsonObject update = params.value("update").toObject();
    QString art = update.value("sessionUpdate").toString();
    if (art == "agent_message_chunk") {
        QJsonObject content = update.value("content").toObject();
        m_samtale->addChunk(content.value("text").toString());
    } else if (art == "tool_call") {
        m_vaerktoejskald->addToolCall(update.value("toolCallId").toString(),
                                      update.value("title").toString(),
                                      update.value("status").toString());
    } else if (art == "tool_call_update") {
        m_vaerktoejskald->updateToolCall(update.value("toolCallId").toString(),
                                         update.value("status").toString(),
                                         update.value("title").toString());
    } else if (art == "turn_end") {
        m_samtale->endRunde();
        emit rundeAfsluttet();
    }
}

void AcpClient::modtagAgentFejl(const QString& fejl)
{
    m_lastError = fejl;
    emit agentFejl(fejl);
}
