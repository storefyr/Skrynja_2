#include "selvtest.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QVariantList>
#include <QVariantMap>

#include "acpclient.h"
#include "diffmodel.h"

SelfTest::SelfTest(AcpClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_verdictWritten(false)
    , m_promptSent(false)
    , m_permissionAnswered(false)
    , m_fileWritten(false)
{
}

void SelfTest::start(const QString& prompt, const QString& verdictPath)
{
    m_prompt = prompt;
    m_verdictPath = verdictPath;
    connect(m_client, SIGNAL(agentReady()), this, SLOT(onAgentReady()));
    connect(m_client, SIGNAL(sessionCreated(QString)), this, SLOT(onSessionCreated(QString)));
    connect(m_client, SIGNAL(permissionRequested()), this, SLOT(onPermissionRequested()));
    connect(m_client, SIGNAL(roundFinished()), this, SLOT(onRoundFinished()));
    connect(m_client, SIGNAL(agentError(QString)), this, SLOT(onAgentError(QString)));
    connect(m_client, SIGNAL(agentDied(int)), this, SLOT(onAgentDied(int)));
    connect(m_client, SIGNAL(messageSent(QByteArray)), this, SLOT(onMessageSent(QByteArray)));
    connect(m_client, SIGNAL(diffsChanged()), this, SLOT(onDiffsChanged()));
    m_client->startAgent();
}

bool SelfTest::verdictWritten() const
{
    return m_verdictWritten;
}

QJsonObject SelfTest::verdict() const
{
    QFile f(m_verdictPath);
    if (!f.open(QIODevice::ReadOnly))
        return QJsonObject();
    return QJsonDocument::fromJson(f.readAll()).object();
}

QString SelfTest::verdictPath() const
{
    return m_verdictPath;
}

void SelfTest::onAgentReady()
{
    m_client->startSession();
}

void SelfTest::onSessionCreated(const QString& sessionId)
{
    Q_UNUSED(sessionId);
    m_client->sendSessionPrompt(m_prompt);
}

void SelfTest::onMessageSent(const QByteArray& line)
{
    if (line.contains("\"session/prompt\""))
        m_promptSent = true;
}

void SelfTest::onPermissionRequested()
{
    QVariantMap pending = m_client->pendingPermission();
    QVariantList options = pending.value("options").toList();
    if (!options.isEmpty()) {
        QString optionId = options.at(0).toMap().value("optionId").toString();
        if (m_client->answerPermission(optionId))
            m_permissionAnswered = true;
    }
}

void SelfTest::onDiffsChanged()
{
    if (m_client->diffModel()->diffCount() > 0)
        m_fileWritten = true;
}

void SelfTest::onRoundFinished()
{
    if (m_promptSent && m_permissionAnswered && m_fileWritten)
        finish(true, "prompt sendt, tilladelse besvaret, fil skrevet, end_turn modtaget");
    else
        finish(false, QString("kæde ufuldstændig: prompt=%1 tilladelse=%2 fil=%3")
               .arg(m_promptSent).arg(m_permissionAnswered).arg(m_fileWritten));
}

void SelfTest::onAgentError(const QString& error)
{
    finish(false, "startfejl/agentfejl: " + error);
}

void SelfTest::onAgentDied(int kode)
{
    finish(false, QString("agentdød (exit %1)").arg(kode));
}

void SelfTest::finish(bool pass, const QString& reason)
{
    if (m_verdictWritten)
        return;
    writeVerdict(pass, reason);
    m_client->stopAgent();
    emit finished(pass ? 0 : 1);
}

void SelfTest::writeVerdict(bool pass, const QString& reason)
{
    if (m_verdictWritten)
        return;
    m_verdictWritten = true;
    QJsonObject o;
    o.insert("resultat", pass ? QString("pass") : QString("fail"));
    o.insert("årsag", reason);
    o.insert("tidsstempel", QDateTime::currentDateTime().toString(Qt::ISODate));
    QDir().mkpath(QFileInfo(m_verdictPath).absolutePath());
    QFile f(m_verdictPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        f.write(QJsonDocument(o).toJson(QJsonDocument::Compact));
        f.close();
    }
}
