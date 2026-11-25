#ifndef KONFIGURATION_H
#define KONFIGURATION_H

#include <QObject>
#include <QString>

class Konfiguration : public QObject
{
    Q_OBJECT
public:
    static Konfiguration* instance();

    QString agentPath() const;
    void setAgentPath(const QString& path);

    QString sessionId() const;
    void setSessionId(const QString& id);
    bool hasSessionId() const;

signals:
    void agentPathChanged();
    void sessionIdChanged();

private:
    explicit Konfiguration(QObject* parent = 0);

    static Konfiguration* s_instance;
};

#endif
