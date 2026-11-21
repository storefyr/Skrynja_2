#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include <QObject>
#include <QString>

class ConversationModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastText READ lastText NOTIFY changed)
    Q_PROPERTY(int round READ round NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
public:
    explicit ConversationModel(QObject* parent = 0);

    void addChunk(const QString& text);
    void startRound();
    void endRound();

    QString lastText() const;
    int round() const;
    QString status() const;

signals:
    void changed();

private:
    QString m_lastText;
    int m_round;
    QString m_status;
};

#endif
