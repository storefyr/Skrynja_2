#include "tilladelsesmodel.h"

#include <QJsonArray>
#include <QJsonObject>

Tilladelsesmodel::Tilladelsesmodel(QObject* parent)
    : QObject(parent)
    , m_requestId(0)
    , m_afventende(false)
    , m_svaret(false)
    , m_antal(0)
{
}

void Tilladelsesmodel::setAnmodning(qint64 requestId, const QJsonObject& params)
{
    m_requestId = requestId;
    m_pending.clear();
    m_pending.insert("requestId", QVariant::fromValue<qint64>(requestId));
    m_pending.insert("toolCall", params.value("toolCall").toObject().toVariantMap());
    QJsonArray options = params.value("options").toArray();
    QVariantList liste;
    for (int i = 0; i < options.size(); ++i)
        liste.append(options.at(i).toObject().toVariantMap());
    m_pending.insert("options", liste);
    m_afventende = true;
    m_svaret = false;
    ++m_antal;
    emit changed();
}

void Tilladelsesmodel::setSvaret()
{
    m_svaret = true;
    m_afventende = false;
    emit changed();
}

void Tilladelsesmodel::clear()
{
    m_pending.clear();
    m_afventende = false;
    m_requestId = 0;
    emit changed();
}

QVariantMap Tilladelsesmodel::pendingPermission() const
{
    return m_pending;
}

qint64 Tilladelsesmodel::anmodningsId() const
{
    return m_requestId;
}

bool Tilladelsesmodel::afventende() const
{
    return m_afventende && !m_svaret;
}

bool Tilladelsesmodel::svaret() const
{
    return m_svaret;
}

int Tilladelsesmodel::anmodninger() const
{
    return m_antal;
}
