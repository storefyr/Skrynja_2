#ifndef ACPBESKED_H
#define ACPBESKED_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

class AcpBesked
{
public:
    enum Type { Ugyldig = 0, Anmodning, Svar, Notifikation };

    AcpBesked();

    static AcpBesked fraJson(const QByteArray& json, bool* ok = 0);
    QByteArray tilJson() const;

    Type type() const;
    bool harId() const;
    qint64 id() const;
    QString metode() const;
    QJsonObject params() const;
    QJsonValue resultat() const;
    bool erFejl() const;
    QJsonObject fejl() const;

    static AcpBesked anmodning(qint64 id, const QString& metode, const QJsonObject& params);
    static AcpBesked svar(qint64 id, const QJsonValue& resultat);
    static AcpBesked fejlSvar(qint64 id, int kode, const QString& besked);
    static AcpBesked notifikation(const QString& metode, const QJsonObject& params);

private:
    Type m_type;
    qint64 m_id;
    bool m_harId;
    QString m_metode;
    QJsonObject m_params;
    QJsonValue m_resultat;
    bool m_erFejl;
    QJsonObject m_fejl;
};

#endif
