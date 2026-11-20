#include "acpbesked.h"

#include <QJsonDocument>

AcpBesked::AcpBesked()
    : m_type(Ugyldig)
    , m_id(0)
    , m_harId(false)
    , m_erFejl(false)
{
}

AcpBesked AcpBesked::fraJson(const QByteArray& json, bool* ok)
{
    if (ok)
        *ok = false;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return AcpBesked();
    QJsonObject obj = doc.object();
    AcpBesked b;
    if (obj.contains("method")) {
        b.m_metode = obj.value("method").toString();
        if (obj.contains("id")) {
            b.m_type = Anmodning;
            b.m_harId = true;
            b.m_id = obj.value("id").toVariant().toLongLong();
        } else {
            b.m_type = Notifikation;
        }
        b.m_params = obj.value("params").toObject();
    } else if (obj.contains("id")) {
        b.m_type = Svar;
        b.m_harId = true;
        b.m_id = obj.value("id").toVariant().toLongLong();
        if (obj.contains("error")) {
            b.m_erFejl = true;
            b.m_fejl = obj.value("error").toObject();
        } else {
            b.m_resultat = obj.value("result");
        }
    } else {
        return AcpBesked();
    }
    if (ok)
        *ok = true;
    return b;
}

QByteArray AcpBesked::tilJson() const
{
    QJsonObject obj;
    obj.insert("jsonrpc", "2.0");
    if (m_harId)
        obj.insert("id", QJsonValue(m_id));
    if (m_type == Anmodning || m_type == Notifikation) {
        obj.insert("method", m_metode);
        obj.insert("params", m_params);
    } else if (m_type == Svar) {
        if (m_erFejl)
            obj.insert("error", m_fejl);
        else
            obj.insert("result", m_resultat);
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

AcpBesked::Type AcpBesked::type() const
{
    return m_type;
}

bool AcpBesked::harId() const
{
    return m_harId;
}

qint64 AcpBesked::id() const
{
    return m_id;
}

QString AcpBesked::metode() const
{
    return m_metode;
}

QJsonObject AcpBesked::params() const
{
    return m_params;
}

QJsonValue AcpBesked::resultat() const
{
    return m_resultat;
}

bool AcpBesked::erFejl() const
{
    return m_erFejl;
}

QJsonObject AcpBesked::fejl() const
{
    return m_fejl;
}

AcpBesked AcpBesked::anmodning(qint64 id, const QString& metode, const QJsonObject& params)
{
    AcpBesked b;
    b.m_type = Anmodning;
    b.m_harId = true;
    b.m_id = id;
    b.m_metode = metode;
    b.m_params = params;
    return b;
}

AcpBesked AcpBesked::svar(qint64 id, const QJsonValue& resultat)
{
    AcpBesked b;
    b.m_type = Svar;
    b.m_harId = true;
    b.m_id = id;
    b.m_resultat = resultat;
    return b;
}

AcpBesked AcpBesked::fejlSvar(qint64 id, int kode, const QString& besked)
{
    AcpBesked b;
    b.m_type = Svar;
    b.m_harId = true;
    b.m_id = id;
    b.m_erFejl = true;
    b.m_fejl.insert("code", kode);
    b.m_fejl.insert("message", besked);
    return b;
}

AcpBesked AcpBesked::notifikation(const QString& metode, const QJsonObject& params)
{
    AcpBesked b;
    b.m_type = Notifikation;
    b.m_metode = metode;
    b.m_params = params;
    return b;
}
