#ifndef SESSIONLAGER_H
#define SESSIONLAGER_H

#include <QString>
#include <QSettings>

class SessionLager
{
public:
    SessionLager();

    void gemSessionId(const QString& id);
    QString sessionId() const;
    bool harSessionId() const;
    void ryd();

private:
    QSettings m_indstillinger;
};

#endif
