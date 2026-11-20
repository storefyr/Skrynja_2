#ifndef SELVTEST_H
#define SELVTEST_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class AcpClient;

class Selvtest : public QObject
{
    Q_OBJECT
public:
    explicit Selvtest(AcpClient* klient, QObject* parent = 0);

    void start(const QString& prompt, const QString& verdictSti);
    bool verdictSkrevet() const;
    QJsonObject verdict() const;
    QString verdictSti() const;

signals:
    void faerdig(int exitKode);

private slots:
    void onAgentKlar();
    void onSessionOprettet(const QString& sessionId);
    void onPermissionAnmodet();
    void onRundeAfsluttet();
    void onAgentFejl(const QString& fejl);
    void onAgentDoed(int kode);
    void onBeskedSendt(const QByteArray& linje);
    void onDiffsAendret();

private:
    void skrivVerdict(bool pass, const QString& årsag);
    void afslut(bool pass, const QString& årsag);

    AcpClient* m_klient;
    QString m_prompt;
    QString m_verdictSti;
    bool m_verdictSkrevet;
    bool m_promptSendt;
    bool m_tilladelseBesvaret;
    bool m_filSkrevet;
};

#endif
