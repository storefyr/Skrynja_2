#ifndef TOOLCALLMODEL_H
#define TOOLCALLMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class ToolCallModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList toolCalls READ toolCalls NOTIFY changed)
public:
    explicit ToolCallModel(QObject* parent = 0);

    void addToolCall(const QString& toolCallId, const QString& title, const QString& status);
    void updateToolCall(const QString& toolCallId, const QString& status, const QString& title = QString());
    bool acknowledge(const QString& toolCallId);

    QVariantList toolCalls() const;
    QVariantMap find(const QString& toolCallId) const;
    int count() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

signals:
    void changed();

private:
    struct Call {
        QString toolCallId;
        QString title;
        QString status;
        bool set;
    };
    QList<Call> m_calls;
};

#endif
