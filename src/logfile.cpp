#include "logfile.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

LogFile* LogFile::instance()
{
    static LogFile g_instans;
    return &g_instans;
}

LogFile::LogFile(QObject* parent)
    : QObject(parent)
    , m_customPath(false)
{
}

QString LogFile::logPath() const
{
    if (m_customPath)
        return m_logPath;
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::homePath() + "/.local/share/harbour-skrynja-2";
    return base + "/skrynja-2.log";
}

void LogFile::setLogPath(const QString& path)
{
    m_logPath = path;
    m_customPath = true;
}

bool LogFile::writeLine(const QString& event)
{
    if (event.isEmpty())
        return false;
    if (event.contains('\n') || event.contains('\r'))
        return false;
    QString path = logPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << event << "\n";
    return true;
}
