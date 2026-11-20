#ifndef VAERKTOEJSKALDSMODEL_H
#define VAERKTOEJSKALDSMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class VaerktoejskaldsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList toolCalls READ toolCalls NOTIFY changed)
public:
    explicit VaerktoejskaldsModel(QObject* parent = 0);

    void addToolCall(const QString& toolCallId, const QString& titel, const QString& status);
    void updateToolCall(const QString& toolCallId, const QString& status, const QString& titel = QString());
    bool acknowledge(const QString& toolCallId);

    QVariantList toolCalls() const;
    QVariantMap find(const QString& toolCallId) const;
    int count() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

signals:
    void changed();

private:
    struct Kald {
        QString toolCallId;
        QString titel;
        QString status;
        bool set;
    };
    QList<Kald> m_kald;
};

#endif
