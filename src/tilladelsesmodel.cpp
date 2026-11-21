#include "tilladelsesmodel.h"

#include <QJsonArray>
#include <QJsonObject>

PermissionModel::PermissionModel(QObject* parent)
    : QObject(parent)
    , m_requestId(0)
    , m_isPending(false)
    , m_answered(false)
    , m_count(0)
{
}

void PermissionModel::setRequest(qint64 requestId, const QJsonObject& params)
{
    m_requestId = requestId;
    m_pending.clear();
    m_pending.insert("requestId", QVariant::fromValue<qint64>(requestId));
    m_pending.insert("toolCall", params.value("toolCall").toObject().toVariantMap());
    QJsonArray options = params.value("options").toArray();
    QVariantList list;
    for (int i = 0; i < options.size(); ++i)
        list.append(options.at(i).toObject().toVariantMap());
    m_pending.insert("options", list);
    m_isPending = true;
    m_answered = false;
    ++m_count;
    emit changed();
}

void PermissionModel::setAnswered()
{
    m_answered = true;
    m_isPending = false;
    emit changed();
}

QVariantMap PermissionModel::pendingPermission() const
{
    return m_pending;
}

qint64 PermissionModel::requestId() const
{
    return m_requestId;
}

bool PermissionModel::isPending() const
{
    return m_isPending && !m_answered;
}

int PermissionModel::requests() const
{
    return m_count;
}
