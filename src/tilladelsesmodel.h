#ifndef TILLADELSESMODEL_H
#define TILLADELSESMODEL_H

#include <QObject>
#include <QVariantMap>

class Tilladelsesmodel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap pendingPermission READ pendingPermission NOTIFY changed)
    Q_PROPERTY(int anmodninger READ anmodninger NOTIFY changed)
public:
    explicit Tilladelsesmodel(QObject* parent = 0);

    void setAnmodning(qint64 requestId, const QJsonObject& params);
    void setSvaret();
    void clear();

    QVariantMap pendingPermission() const;
    qint64 anmodningsId() const;
    bool afventende() const;
    bool svaret() const;
    int anmodninger() const;

signals:
    void changed();

private:
    qint64 m_requestId;
    QVariantMap m_pending;
    bool m_afventende;
    bool m_svaret;
    int m_antal;
};

#endif
