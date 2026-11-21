#include "acpmessage.h"

#include <QJsonDocument>
#include <QVariant>

AcpMessage::AcpMessage()
    : m_type(Invalid)
    , m_id(0)
    , m_hasId(false)
    , m_isError(false)
{
}

AcpMessage AcpMessage::fraJson(const QByteArray& json, bool* ok)
{
    if (ok)
        *ok = false;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return AcpMessage();
    QJsonObject obj = doc.object();
    AcpMessage b;
    if (obj.contains("method")) {
        b.m_method = obj.value("method").toString();
        if (obj.contains("id")) {
            b.m_type = Request;
            b.m_hasId = true;
            b.m_id = obj.value("id").toVariant().toLongLong();
        } else {
            b.m_type = Notification;
        }
        b.m_params = obj.value("params").toObject();
    } else if (obj.contains("id")) {
        b.m_type = Response;
        b.m_hasId = true;
        b.m_id = obj.value("id").toVariant().toLongLong();
        if (obj.contains("error")) {
            b.m_isError = true;
            b.m_error = obj.value("error").toObject();
        } else {
            b.m_result = obj.value("result");
        }
    } else {
        return AcpMessage();
    }
    if (ok)
        *ok = true;
    return b;
}

QByteArray AcpMessage::tilJson() const
{
    QJsonObject obj;
    obj.insert("jsonrpc", "2.0");
    if (m_hasId)
        obj.insert("id", QJsonValue(m_id));
    if (m_type == Request || m_type == Notification) {
        obj.insert("method", m_method);
        obj.insert("params", m_params);
    } else if (m_type == Response) {
        if (m_isError)
            obj.insert("error", m_error);
        else
            obj.insert("result", m_result);
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

AcpMessage::Type AcpMessage::type() const
{
    return m_type;
}

bool AcpMessage::hasId() const
{
    return m_hasId;
}

qint64 AcpMessage::id() const
{
    return m_id;
}

QString AcpMessage::method() const
{
    return m_method;
}

QJsonObject AcpMessage::params() const
{
    return m_params;
}

QJsonValue AcpMessage::result() const
{
    return m_result;
}

bool AcpMessage::isError() const
{
    return m_isError;
}

QJsonObject AcpMessage::error() const
{
    return m_error;
}

AcpMessage AcpMessage::request(qint64 id, const QString& method, const QJsonObject& params)
{
    AcpMessage b;
    b.m_type = Request;
    b.m_hasId = true;
    b.m_id = id;
    b.m_method = method;
    b.m_params = params;
    return b;
}

AcpMessage AcpMessage::response(qint64 id, const QJsonValue& result)
{
    AcpMessage b;
    b.m_type = Response;
    b.m_hasId = true;
    b.m_id = id;
    b.m_result = result;
    return b;
}

AcpMessage AcpMessage::errorResponse(qint64 id, int kode, const QString& message)
{
    AcpMessage b;
    b.m_type = Response;
    b.m_hasId = true;
    b.m_id = id;
    b.m_isError = true;
    b.m_error.insert("code", kode);
    b.m_error.insert("message", message);
    return b;
}

AcpMessage AcpMessage::notification(const QString& method, const QJsonObject& params)
{
    AcpMessage b;
    b.m_type = Notification;
    b.m_method = method;
    b.m_params = params;
    return b;
}
