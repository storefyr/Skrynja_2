#ifndef ACPMESSAGE_H
#define ACPMESSAGE_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

class AcpMessage
{
public:
    enum Type { Invalid = 0, Request, Response, Notification };

    AcpMessage();

    static AcpMessage fraJson(const QByteArray& json, bool* ok = 0);
    QByteArray tilJson() const;

    Type type() const;
    bool hasId() const;
    qint64 id() const;
    QString method() const;
    QJsonObject params() const;
    QJsonValue result() const;
    bool isError() const;
    QJsonObject error() const;

    static AcpMessage request(qint64 id, const QString& method, const QJsonObject& params);
    static AcpMessage response(qint64 id, const QJsonValue& result);
    static AcpMessage errorResponse(qint64 id, int kode, const QString& message);
    static AcpMessage notification(const QString& method, const QJsonObject& params);

private:
    Type m_type;
    qint64 m_id;
    bool m_hasId;
    QString m_method;
    QJsonObject m_params;
    QJsonValue m_result;
    bool m_isError;
    QJsonObject m_error;
};

#endif
