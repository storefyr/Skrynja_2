#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

#include <QString>

class SessionStore
{
public:
    SessionStore();

    void saveSessionId(const QString& id);
    QString sessionId() const;
    bool hasSessionId() const;
};

#endif
