#include "diffmodel.h"

#include "acpprotokol.h"

DiffModel::DiffModel(QObject* parent)
    : QObject(parent)
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
    m_diffs.append(d);
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
