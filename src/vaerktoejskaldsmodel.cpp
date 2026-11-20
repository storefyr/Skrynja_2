#include "vaerktoejskaldsmodel.h"

VaerktoejskaldsModel::VaerktoejskaldsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void VaerktoejskaldsModel::addToolCall(const QString& toolCallId, const QString& titel, const QString& status)
{
    Kald k;
    k.toolCallId = toolCallId;
    k.titel = titel;
    k.status = status.isEmpty() ? QString("pending") : status;
    k.set = false;
    beginInsertRows(QModelIndex(), m_kald.size(), m_kald.size());
    m_kald.append(k);
    endInsertRows();
    emit changed();
}

void VaerktoejskaldsModel::updateToolCall(const QString& toolCallId, const QString& status, const QString& titel)
{
    for (int i = 0; i < m_kald.size(); ++i) {
        if (m_kald.at(i).toolCallId == toolCallId) {
            if (!status.isEmpty())
                m_kald[i].status = status;
            if (!titel.isEmpty())
                m_kald[i].titel = titel;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx);
            emit changed();
            return;
        }
    }
}

bool VaerktoejskaldsModel::acknowledge(const QString& toolCallId)
{
    for (int i = 0; i < m_kald.size(); ++i) {
        if (m_kald.at(i).toolCallId == toolCallId) {
            if (m_kald.at(i).set)
                return false;
            m_kald[i].set = true;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx);
            emit changed();
            return true;
        }
    }
    return false;
}

QVariantList VaerktoejskaldsModel::toolCalls() const
{
    QVariantList liste;
    for (int i = 0; i < m_kald.size(); ++i) {
        QVariantMap m;
        m.insert("toolCallId", m_kald.at(i).toolCallId);
        m.insert("titel", m_kald.at(i).titel);
        m.insert("status", m_kald.at(i).status);
        m.insert("set", m_kald.at(i).set);
        liste.append(m);
    }
    return liste;
}

QVariantMap VaerktoejskaldsModel::find(const QString& toolCallId) const
{
    for (int i = 0; i < m_kald.size(); ++i) {
        if (m_kald.at(i).toolCallId == toolCallId) {
            QVariantMap m;
            m.insert("toolCallId", m_kald.at(i).toolCallId);
            m.insert("titel", m_kald.at(i).titel);
            m.insert("status", m_kald.at(i).status);
            m.insert("set", m_kald.at(i).set);
            return m;
        }
    }
    return QVariantMap();
}

int VaerktoejskaldsModel::count() const
{
    return m_kald.size();
}

int VaerktoejskaldsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_kald.size();
}

QVariant VaerktoejskaldsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_kald.size())
        return QVariant();
    const Kald& k = m_kald.at(index.row());
    if (role == Qt::DisplayRole)
        return k.titel + " (" + k.status + ")";
    if (role == Qt::UserRole + 1)
        return k.toolCallId;
    if (role == Qt::UserRole + 2)
        return k.status;
    return QVariant();
}
