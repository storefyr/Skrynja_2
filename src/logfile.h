#ifndef LOGFILE_H
#define LOGFILE_H

#include <QObject>
#include <QString>

class LogFile : public QObject
{
    Q_OBJECT
public:
    static LogFile* instance();

    QString logPath() const;
    bool writeLine(const QString& event);
    void setLogPath(const QString& path);

private:
    explicit LogFile(QObject* parent = 0);

    QString m_logPath;
    bool m_customPath;
};

#endif
