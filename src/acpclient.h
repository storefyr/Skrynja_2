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
class DiffModel;
class Konfigurationsmodel;
class SamtaleModel;
class SessionLager;
class Tilladelsesmodel;
class VaerktoejskaldsModel;

class AcpClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hadRestoredHistory READ hadRestoredHistory NOTIFY historieAendret)
    Q_PROPERTY(QVariantList toolkald READ toolkald NOTIFY vaerktoejskaldAendret)
    Q_PROPERTY(QString samtaleTekst READ samtaleTekst NOTIFY samtaleAendret)
    Q_PROPERTY(int samtaleRunde READ samtaleRunde NOTIFY samtaleAendret)
    Q_PROPERTY(QString samtaleStatus READ samtaleStatus NOTIFY samtaleAendret)
    Q_PROPERTY(QVariantMap pendingPermission READ pendingPermission NOTIFY tilladelseAendret)
    Q_PROPERTY(int tilladelseAntal READ tilladelseAntal NOTIFY tilladelseAendret)
    Q_PROPERTY(QVariantList diffs READ diffs NOTIFY diffsAendret)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY outputsAendret)
    Q_PROPERTY(QVariantList configOptions READ configOptions NOTIFY konfigurationAendret)
    Q_PROPERTY(QString agentsti READ agentsti NOTIFY agentstiAendret)
public:
    explicit AcpClient(QObject* parent = 0);
    ~AcpClient();

    bool startAgent();
    void sendInitialize();
    QString lastError() const;
    AgentProces* agentProces() const;
    QVariantList configOptions() const;

    Q_INVOKABLE void startSession();
    Q_INVOKABLE void sendSessionNew(const QString& cwd);
    Q_INVOKABLE void sendSessionPrompt(const QString& prompt);
    SamtaleModel* samtaleModel() const;
    VaerktoejskaldsModel* vaerktoejskaldsModel() const;
    QVariantList toolkald() const;
    QString samtaleTekst() const;
    int samtaleRunde() const;
    QString samtaleStatus() const;
    Q_INVOKABLE void kvitterVaerktoejskald(const QString& toolCallId);
    Q_INVOKABLE bool answerPermission(const QString& optionId);
    Q_INVOKABLE bool afvisPermission();
    Q_INVOKABLE void sessionCancel();
    Q_INVOKABLE void stopAgent();
    Tilladelsesmodel* tilladelsesmodel() const;
    QVariantMap pendingPermission() const;
    int tilladelseAntal() const;
    DiffModel* diffModel() const;
    QVariantList diffs() const;
    QVariantList outputs() const;
    Konfigurationsmodel* konfigurationsmodel() const;
    Q_INVOKABLE void setConfigOption(const QString& configId, const QVariant& værdi);
    QString agentsti() const;
    Q_INVOKABLE void setAgentsti(const QString& sti);
    QString sessionId() const;
    QString stopReason() const;
    bool hadRestoredHistory() const;

    void sætTestTilstand(bool tilstand);
    bool testTilstand() const;
    void feedBytesForTest(const QByteArray& data);
    void simulerAgentDødForTest(int kode);
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
    void historieAendret();
    void vaerktoejskaldAendret();
    void samtaleAendret();
    void permissionAnmodet();
    void tilladelseAendret();
    void agentDoed(int kode);
    void diffsAendret();
    void outputsAendret();
    void konfigurationAendret();
    void agentstiAendret();

protected:
    qint64 næsteId();
    qint64 sendRequest(const QString& metode, const QJsonObject& params);
    void sendSessionLoad(const QString& sessionId);
    void sendNotifikation(const QString& metode, const QJsonObject& params);
    void sendBesked(const AcpBesked& besked);
    void skriv(const QByteArray& linje);
    virtual void håndterBesked(const AcpBesked& besked);
    void håndterNotifikation(const AcpBesked& besked);
    void håndterSessionUpdate(const QJsonObject& params);
    void håndterContentBlokke(const QJsonObject& update);

    AgentProces* m_proces;
    AcpFrameReader* m_læser;
    SamtaleModel* m_samtale;
    SessionLager* m_lager;
    VaerktoejskaldsModel* m_vaerktoejskald;
    Tilladelsesmodel* m_tilladelse;
    DiffModel* m_diff;
    Konfigurationsmodel* m_konfiguration;
    qint64 m_næsteId;
    bool m_testTilstand;
    QList<QByteArray> m_sendte;
    QHash<qint64, QString> m_åbneAnmodninger;
    QString m_lastError;
    QString m_sessionId;
    QString m_stopReason;
    bool m_hadRestoredHistory;
    QVariantList m_configOptions;

private slots:
    void modtagLinje(const QByteArray& linje);
    void modtagAgentFejl(const QString& fejl);
    void procesFaerdig(int exitKode);
};

#endif
