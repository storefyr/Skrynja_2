#include "samtalemodel.h"

ConversationModel::ConversationModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_round(0)
    , m_status("klar")
{
}

void ConversationModel::addChunk(const QString& text)
{
    beginInsertRows(QModelIndex(), m_lines.size(), m_lines.size());
    m_lines.append(text);
    endInsertRows();
    m_lastText += text;
    emit changed();
}

void ConversationModel::startRound()
{
    ++m_round;
    m_lastText.clear();
    m_status = "kører";
    emit changed();
}

void ConversationModel::endRound()
{
    m_status = "klar";
    emit changed();
    emit roundFinished();
}

QString ConversationModel::lastText() const
{
    return m_lastText;
}

int ConversationModel::round() const
{
    return m_round;
}

QString ConversationModel::status() const
{
    return m_status;
}

int ConversationModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_lines.size();
}

QVariant ConversationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_lines.size())
        return QVariant();
    if (role == TextRole || role == Qt::DisplayRole)
        return m_lines.at(index.row());
    return QVariant();
}
