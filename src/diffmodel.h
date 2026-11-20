#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class DiffModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList diffs READ diffs NOTIFY changed)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY changed)
public:
    explicit DiffModel(QObject* parent = 0);

    void addDiff(const QString& sti, const QString& diffTekst);
    void addOutput(const QString& tekst);

    QVariantList diffs() const;
    QVariantList outputs() const;
    int diffCount() const;
    int outputCount() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

signals:
    void changed();

private:
    struct Diff {
        QString sti;
        QStringList linjer;
    };
    QList<Diff> m_diffs;
    QStringList m_outputs;
};

#endif
