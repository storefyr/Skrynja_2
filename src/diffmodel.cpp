#include "diffmodel.h"

#include "acpframereader.h"

DiffModel::DiffModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void DiffModel::addDiff(const QString& sti, const QString& diffTekst)
{
    Diff d;
    d.sti = sti;
    QStringList linjer = diffTekst.split('\n', QString::KeepEmptyParts);
    int cap = AcpFrameReader::standardCap();
    for (int i = 0; i < linjer.size(); ++i) {
        if (linjer.at(i).size() > cap)
            linjer[i] = linjer.at(i).left(cap);
    }
    d.linjer = linjer;
    beginInsertRows(QModelIndex(), m_diffs.size(), m_diffs.size());
    m_diffs.append(d);
    endInsertRows();
    emit changed();
}

void DiffModel::addOutput(const QString& tekst)
{
    m_outputs.append(tekst);
    emit changed();
}

QVariantList DiffModel::diffs() const
{
    QVariantList liste;
    for (int i = 0; i < m_diffs.size(); ++i) {
        QVariantMap m;
        m.insert("sti", m_diffs.at(i).sti);
        m.insert("linjer", m_diffs.at(i).linjer);
        m.insert("tekst", m_diffs.at(i).linjer.join("\n"));
        liste.append(m);
    }
    return liste;
}

QVariantList DiffModel::outputs() const
{
    QVariantList liste;
    for (int i = 0; i < m_outputs.size(); ++i)
        liste.append(m_outputs.at(i));
    return liste;
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
        return m_diffs.at(index.row()).sti;
    if (role == Qt::UserRole + 1)
        return m_diffs.at(index.row()).linjer.join("\n");
    return QVariant();
}
