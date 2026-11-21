#include "toolcallmodel.h"

ToolCallModel::ToolCallModel(QObject* parent)
    : QObject(parent)
{
}

void ToolCallModel::addToolCall(const QString& toolCallId, const QString& title, const QString& status)
{
    Call k;
    k.toolCallId = toolCallId;
    k.title = title;
    k.status = status.isEmpty() ? QString("pending") : status;
    k.set = false;
    m_calls.append(k);
    emit changed();
}

void ToolCallModel::updateToolCall(const QString& toolCallId, const QString& status, const QString& title)
{
    for (int i = 0; i < m_calls.size(); ++i) {
        if (m_calls.at(i).toolCallId == toolCallId) {
            if (!status.isEmpty())
                m_calls[i].status = status;
            if (!title.isEmpty())
                m_calls[i].title = title;
            emit changed();
            return;
        }
    }
}

bool ToolCallModel::acknowledge(const QString& toolCallId)
{
    for (int i = 0; i < m_calls.size(); ++i) {
        if (m_calls.at(i).toolCallId == toolCallId) {
            if (m_calls.at(i).set)
                return false;
            m_calls[i].set = true;
            emit changed();
            return true;
        }
    }
    return false;
}

QVariantMap ToolCallModel::projektion(const Call& k) const
{
    QVariantMap m;
    m.insert("toolCallId", k.toolCallId);
    m.insert("title", k.title);
    m.insert("status", k.status);
    m.insert("set", k.set);
    return m;
}

QVariantList ToolCallModel::toolCalls() const
{
    QVariantList list;
    for (int i = 0; i < m_calls.size(); ++i)
        list.append(projektion(m_calls.at(i)));
    return list;
}

QVariantMap ToolCallModel::find(const QString& toolCallId) const
{
    for (int i = 0; i < m_calls.size(); ++i) {
        if (m_calls.at(i).toolCallId == toolCallId)
            return projektion(m_calls.at(i));
    }
    return QVariantMap();
}

int ToolCallModel::count() const
{
    return m_calls.size();
}
