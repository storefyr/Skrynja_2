#include "sessionstore.h"

SessionStore::SessionStore()
    : m_settings()
{
}

void SessionStore::saveSessionId(const QString& id)
{
    m_settings.setValue("session/id", id);
    m_settings.sync();
}

QString SessionStore::sessionId() const
{
    return m_settings.value("session/id").toString();
}

bool SessionStore::hasSessionId() const
{
    return m_settings.contains("session/id");
}
