#ifndef LOGFIL_H
#define LOGFIL_H

#include <QObject>
#include <QString>

class Logfil : public QObject
{
    Q_OBJECT
public:
    static Logfil* instans();

    QString logsti() const;
    bool skrivLinje(const QString& hændelse);
    void sætLogstiForTest(const QString& sti);

private:
    explicit Logfil(QObject* parent = 0);

    QString m_logsti;
    bool m_teststi;
};

#endif
