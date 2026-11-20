#include "sessionlager.h"

SessionLager::SessionLager()
    : m_indstillinger()
{
}

void SessionLager::gemSessionId(const QString& id)
{
    m_indstillinger.setValue("session/id", id);
    m_indstillinger.sync();
}

QString SessionLager::sessionId() const
{
    return m_indstillinger.value("session/id").toString();
}

bool SessionLager::harSessionId() const
{
    return m_indstillinger.contains("session/id");
}

void SessionLager::ryd()
{
    m_indstillinger.remove("session/id");
    m_indstillinger.sync();
}
