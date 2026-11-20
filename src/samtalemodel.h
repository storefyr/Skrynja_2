#ifndef SAMTALEMODEL_H
#define SAMTALEMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QStringList>

class SamtaleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString lastText READ lastText NOTIFY changed)
    Q_PROPERTY(int round READ round NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
public:
    enum Roller { TekstRolle = Qt::UserRole + 1 };

    explicit SamtaleModel(QObject* parent = 0);

    void addChunk(const QString& tekst);
    void startRunde();
    void endRunde();

    QString lastText() const;
    int round() const;
    QString status() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

signals:
    void changed();
    void rundeAfsluttet();

private:
    QStringList m_linjer;
    QString m_lastText;
    int m_round;
    QString m_status;
};

#endif
