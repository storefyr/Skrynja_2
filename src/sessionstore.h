#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

#include <QString>
#include <QSettings>

class SessionStore
{
public:
    SessionStore();

    void saveSessionId(const QString& id);
    QString sessionId() const;
    bool hasSessionId() const;

private:
    QSettings m_settings;
};

#endif
