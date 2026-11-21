#include "sessionstore.h"

#include "konfiguration.h"

SessionStore::SessionStore()
{
}

void SessionStore::saveSessionId(const QString& id)
{
    Konfiguration::instance()->setSessionId(id);
}

QString SessionStore::sessionId() const
{
    return Konfiguration::instance()->sessionId();
}

bool SessionStore::hasSessionId() const
{
    return Konfiguration::instance()->hasSessionId();
}
