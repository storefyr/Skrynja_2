#include "konfiguration.h"

#include <QSettings>

Konfiguration* Konfiguration::s_instance = 0;

Konfiguration* Konfiguration::instance()
{
    if (!s_instance)
        s_instance = new Konfiguration;
    return s_instance;
}

Konfiguration::Konfiguration(QObject* parent)
    : QObject(parent)
{
}

QString Konfiguration::agentPath() const
{
    QSettings settings;
    return settings.value("agent/sti", "/home/defaultuser/.local/bin/vibe-acp").toString();
}

void Konfiguration::setAgentPath(const QString& path)
{
    if (agentPath() == path)
        return;
    QSettings settings;
    settings.setValue("agent/sti", path);
    settings.sync();
    emit agentPathChanged();
}

QString Konfiguration::sessionId() const
{
    QSettings settings;
    return settings.value("session/id").toString();
}

void Konfiguration::setSessionId(const QString& id)
{
    if (sessionId() == id)
        return;
    QSettings settings;
    settings.setValue("session/id", id);
    settings.sync();
    emit sessionIdChanged();
}

bool Konfiguration::hasSessionId() const
{
    QSettings settings;
    return settings.contains("session/id");
}
