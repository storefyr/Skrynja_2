#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class DiffModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList diffs READ diffs NOTIFY changed)
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY changed)
public:
    explicit DiffModel(QObject* parent = 0);

    void addDiff(const QString& path, const QString& diffText);
    void addOutput(const QString& text);

    QVariantList diffs() const;
    QVariantList outputs() const;
    int diffCount() const;
    int outputCount() const;

signals:
    void changed();

private:
    struct Diff {
        QString path;
        QStringList lines;
    };
    QList<Diff> m_diffs;
    QStringList m_outputs;
};

#endif
