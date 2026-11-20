#include "acpclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "acpframereader.h"
#include "agentproces.h"
#include "samtalemodel.h"

static const QString kSessionPrompt = QLatin1String("session/prompt");

AcpClient::AcpClient(QObject* parent)
    : QObject(parent)
    , m_proces(new AgentProces(this))
    , m_læser(new AcpFrameReader(m_proces->proces(), this))
    , m_samtale(new SamtaleModel(this))
    , m_næsteId(1)
    , m_testTilstand(false)
{
    connect(m_læser, SIGNAL(beskedKlar(QByteArray)), this, SLOT(modtagLinje(QByteArray)));
    connect(m_proces, SIGNAL(fejl(QString)), this, SLOT(modtagAgentFejl(QString)));
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

QString AcpClient::sessionId() const
{
    return m_sessionId;
}

QString AcpClient::stopReason() const
{
    return m_stopReason;
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
        } else if (metode == "session/new") {
            m_sessionId = resultat.value("sessionId").toString();
            emit sessionOprettet(m_sessionId);
        } else if (metode == kSessionPrompt) {
            m_stopReason = resultat.value("stopReason").toString();
        }
    } else if (besked.type() == AcpBesked::Notifikation) {
        håndterNotifikation(besked);
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
