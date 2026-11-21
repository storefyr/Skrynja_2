#include "diffmodel.h"

#include "acpprotokol.h"

DiffModel::DiffModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void DiffModel::addDiff(const QString& path, const QString& diffText)
{
    Diff d;
    d.path = path;
    QStringList lines = diffText.split('\n', QString::KeepEmptyParts);
    int cap = AcpProtokol::standardCap();
    for (int i = 0; i < lines.size(); ++i) {
        if (lines.at(i).size() > cap)
            lines[i] = lines.at(i).left(cap);
    }
    d.lines = lines;
    beginInsertRows(QModelIndex(), m_diffs.size(), m_diffs.size());
    m_diffs.append(d);
    endInsertRows();
    emit changed();
}

void DiffModel::addOutput(const QString& text)
{
    m_outputs.append(text);
    emit changed();
}

QVariantList DiffModel::diffs() const
{
    QVariantList list;
    for (int i = 0; i < m_diffs.size(); ++i) {
        QVariantMap m;
        m.insert("path", m_diffs.at(i).path);
        m.insert("lines", m_diffs.at(i).lines);
        m.insert("text", m_diffs.at(i).lines.join("\n"));
        list.append(m);
    }
    return list;
}

QVariantList DiffModel::outputs() const
{
    QVariantList list;
    for (int i = 0; i < m_outputs.size(); ++i)
        list.append(m_outputs.at(i));
    return list;
}

int DiffModel::diffCount() const
{
    return m_diffs.size();
}

int DiffModel::outputCount() const
{
    return m_outputs.size();
}

int DiffModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_diffs.size();
}

QVariant DiffModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_diffs.size())
        return QVariant();
    if (role == Qt::DisplayRole)
        return m_diffs.at(index.row()).path;
    if (role == Qt::UserRole + 1)
        return m_diffs.at(index.row()).lines.join("\n");
    return QVariant();
}
