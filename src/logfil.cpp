#include "logfil.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

Logfil* Logfil::instans()
{
    static Logfil g_instans;
    return &g_instans;
}

Logfil::Logfil(QObject* parent)
    : QObject(parent)
    , m_teststi(false)
{
}

QString Logfil::logsti() const
{
    if (m_teststi)
        return m_logsti;
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::homePath() + "/.local/share/harbour-skrynja-2";
    return base + "/skrynja-2.log";
}

void Logfil::sætLogstiForTest(const QString& sti)
{
    m_logsti = sti;
    m_teststi = true;
}

bool Logfil::skrivLinje(const QString& hændelse)
{
    if (hændelse.isEmpty())
        return false;
    if (hændelse.contains('\n') || hændelse.contains('\r'))
        return false;
    QString sti = logsti();
    QDir().mkpath(QFileInfo(sti).absolutePath());
    QFile fil(sti);
    if (!fil.open(QIODevice::Append | QIODevice::Text))
        return false;
    QTextStream strøm(&fil);
    strøm.setCodec("UTF-8");
    strøm << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << hændelse << "\n";
    return true;
}
