#include "samtalemodel.h"

SamtaleModel::SamtaleModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_round(0)
    , m_status("klar")
{
}

void SamtaleModel::addChunk(const QString& tekst)
{
    beginInsertRows(QModelIndex(), m_linjer.size(), m_linjer.size());
    m_linjer.append(tekst);
    endInsertRows();
    m_lastText += tekst;
    emit changed();
}

void SamtaleModel::startRunde()
{
    ++m_round;
    m_lastText.clear();
    m_status = "kører";
    emit changed();
}

void SamtaleModel::endRunde()
{
    m_status = "klar";
    emit changed();
    emit rundeAfsluttet();
}

QString SamtaleModel::lastText() const
{
    return m_lastText;
}

int SamtaleModel::round() const
{
    return m_round;
}

QString SamtaleModel::status() const
{
    return m_status;
}

int SamtaleModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_linjer.size();
}

QVariant SamtaleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_linjer.size())
        return QVariant();
    if (role == TekstRolle || role == Qt::DisplayRole)
        return m_linjer.at(index.row());
    return QVariant();
}
