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

protected:
    qint64 næsteId();
    qint64 sendRequest(const QString& metode, const QJsonObject& params);
    void sendNotifikation(const QString& metode, const QJsonObject& params);
    void sendBesked(const AcpBesked& besked);
    void skriv(const QByteArray& linje);
    virtual void håndterBesked(const AcpBesked& besked);

    AgentProces* m_proces;
    AcpFrameReader* m_læser;
    qint64 m_næsteId;
    bool m_testTilstand;
    QList<QByteArray> m_sendte;
    QHash<qint64, QString> m_åbneAnmodninger;
    QString m_lastError;
    QVariantList m_configOptions;

private slots:
    void modtagLinje(const QByteArray& linje);
    void modtagAgentFejl(const QString& fejl);
};

#endif
