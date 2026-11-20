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

Selvtest::Selvtest(AcpClient* klient, QObject* parent)
    : QObject(parent)
    , m_klient(klient)
    , m_verdictSkrevet(false)
    , m_promptSendt(false)
    , m_tilladelseBesvaret(false)
    , m_filSkrevet(false)
{
}

void Selvtest::start(const QString& prompt, const QString& verdictSti)
{
    m_prompt = prompt;
    m_verdictSti = verdictSti;
    connect(m_klient, SIGNAL(agentKlar()), this, SLOT(onAgentKlar()));
    connect(m_klient, SIGNAL(sessionOprettet(QString)), this, SLOT(onSessionOprettet(QString)));
    connect(m_klient, SIGNAL(permissionAnmodet()), this, SLOT(onPermissionAnmodet()));
    connect(m_klient, SIGNAL(rundeAfsluttet()), this, SLOT(onRundeAfsluttet()));
    connect(m_klient, SIGNAL(agentFejl(QString)), this, SLOT(onAgentFejl(QString)));
    connect(m_klient, SIGNAL(agentDoed(int)), this, SLOT(onAgentDoed(int)));
    connect(m_klient, SIGNAL(beskedSendt(QByteArray)), this, SLOT(onBeskedSendt(QByteArray)));
    connect(m_klient, SIGNAL(diffsAendret()), this, SLOT(onDiffsAendret()));
    m_klient->startAgent();
}

bool Selvtest::verdictSkrevet() const
{
    return m_verdictSkrevet;
}

QJsonObject Selvtest::verdict() const
{
    QFile f(m_verdictSti);
    if (!f.open(QIODevice::ReadOnly))
        return QJsonObject();
    return QJsonDocument::fromJson(f.readAll()).object();
}

QString Selvtest::verdictSti() const
{
    return m_verdictSti;
}

void Selvtest::onAgentKlar()
{
    m_klient->startSession();
}

void Selvtest::onSessionOprettet(const QString& sessionId)
{
    Q_UNUSED(sessionId);
    m_klient->sendSessionPrompt(m_prompt);
}

void Selvtest::onBeskedSendt(const QByteArray& linje)
{
    if (linje.contains("\"session/prompt\""))
        m_promptSendt = true;
}

void Selvtest::onPermissionAnmodet()
{
    QVariantMap pending = m_klient->pendingPermission();
    QVariantList options = pending.value("options").toList();
    if (!options.isEmpty()) {
        QString optionId = options.at(0).toMap().value("optionId").toString();
        if (m_klient->answerPermission(optionId))
            m_tilladelseBesvaret = true;
    }
}

void Selvtest::onDiffsAendret()
{
    if (m_klient->diffModel()->diffCount() > 0)
        m_filSkrevet = true;
}

void Selvtest::onRundeAfsluttet()
{
    if (m_promptSendt && m_tilladelseBesvaret && m_filSkrevet)
        afslut(true, "prompt sendt, tilladelse besvaret, fil skrevet, end_turn modtaget");
    else
        afslut(false, QString("kæde ufuldstændig: prompt=%1 tilladelse=%2 fil=%3")
               .arg(m_promptSendt).arg(m_tilladelseBesvaret).arg(m_filSkrevet));
}

void Selvtest::onAgentFejl(const QString& fejl)
{
    afslut(false, "startfejl/agentfejl: " + fejl);
}

void Selvtest::onAgentDoed(int kode)
{
    afslut(false, QString("agentdød (exit %1)").arg(kode));
}

void Selvtest::afslut(bool pass, const QString& årsag)
{
    if (m_verdictSkrevet)
        return;
    skrivVerdict(pass, årsag);
    m_klient->stopAgent();
    emit faerdig(pass ? 0 : 1);
}

void Selvtest::skrivVerdict(bool pass, const QString& årsag)
{
    if (m_verdictSkrevet)
        return;
    m_verdictSkrevet = true;
    QJsonObject o;
    o.insert("resultat", pass ? QString("pass") : QString("fail"));
    o.insert("årsag", årsag);
    o.insert("tidsstempel", QDateTime::currentDateTime().toString(Qt::ISODate));
    QDir().mkpath(QFileInfo(m_verdictSti).absolutePath());
    QFile f(m_verdictSti);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        f.write(QJsonDocument(o).toJson(QJsonDocument::Compact));
        f.close();
    }
}
