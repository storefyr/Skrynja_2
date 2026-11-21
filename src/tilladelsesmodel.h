#ifndef PERMISSIONMODEL_H
#define PERMISSIONMODEL_H

#include <QObject>
#include <QVariantMap>

class PermissionModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap pendingPermission READ pendingPermission NOTIFY changed)
    Q_PROPERTY(int requests READ requests NOTIFY changed)
public:
    explicit PermissionModel(QObject* parent = 0);

    void setRequest(qint64 requestId, const QJsonObject& params);
    void setAnswered();
    void clear();

    QVariantMap pendingPermission() const;
    qint64 requestId() const;
    bool isPending() const;
    bool isAnswered() const;
    int requests() const;

signals:
    void changed();

private:
    qint64 m_requestId;
    QVariantMap m_pending;
    bool m_isPending;
    bool m_answered;
    int m_count;
};

#endif
