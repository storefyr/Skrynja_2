#ifndef ACPCLIENT_H
#define ACPCLIENT_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QVariantList>

#include "acpbesked.h"

class AcpFrameReader;
class AgentProces;
class SamtaleModel;

class AcpClient : public QObject
{
    Q_OBJECT
public:
    explicit AcpClient(QObject* parent = 0);
    ~AcpClient();

    bool startAgent();
    void sendInitialize();
    QString lastError() const;
    AgentProces* agentProces() const;
    QVariantList configOptions() const;

    Q_INVOKABLE void sendSessionNew(const QString& cwd);
    Q_INVOKABLE void sendSessionPrompt(const QString& prompt);
    SamtaleModel* samtaleModel() const;
    QString sessionId() const;
    QString stopReason() const;

    void sætTestTilstand(bool tilstand);
    bool testTilstand() const;
    void feedBytesForTest(const QByteArray& data);
    QList<QByteArray> sendteForTest() const;
    QByteArray sidsteSendtForTest() const;
    void nulstilForTest();

signals:
    void beskedSendt(const QByteArray& linje);
    void agentKlar();
    void agentFejl(const QString& fejl);
    void initializeSvar(const QJsonObject& resultat);
    void sessionOprettet(const QString& sessionId);
    void rundeAfsluttet();

protected:
    qint64 næsteId();
    qint64 sendRequest(const QString& metode, const QJsonObject& params);
    void sendNotifikation(const QString& metode, const QJsonObject& params);
    void sendBesked(const AcpBesked& besked);
    void skriv(const QByteArray& linje);
    virtual void håndterBesked(const AcpBesked& besked);
    void håndterNotifikation(const AcpBesked& besked);
    void håndterSessionUpdate(const QJsonObject& params);

    AgentProces* m_proces;
    AcpFrameReader* m_læser;
    SamtaleModel* m_samtale;
    qint64 m_næsteId;
    bool m_testTilstand;
    QList<QByteArray> m_sendte;
    QHash<qint64, QString> m_åbneAnmodninger;
    QString m_lastError;
    QString m_sessionId;
    QString m_stopReason;
    QVariantList m_configOptions;

private slots:
    void modtagLinje(const QByteArray& linje);
    void modtagAgentFejl(const QString& fejl);
};

#endif
