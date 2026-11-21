#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QStringList>

class ConversationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString lastText READ lastText NOTIFY changed)
    Q_PROPERTY(int round READ round NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
public:
    enum Roles { TextRole = Qt::UserRole + 1 };

    explicit ConversationModel(QObject* parent = 0);

    void addChunk(const QString& text);
    void startRound();
    void endRound();

    QString lastText() const;
    int round() const;
    QString status() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

signals:
    void changed();

private:
    QStringList m_lines;
    QString m_lastText;
    int m_round;
    QString m_status;
};

#endif
