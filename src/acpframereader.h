#ifndef ACPFRAMEREADER_H
#define ACPFRAMEREADER_H

#include <QByteArray>
#include <QList>
#include <QObject>

class QIODevice;

class AcpFrameReader : public QObject
{
    Q_OBJECT
public:
    explicit AcpFrameReader(QIODevice* enhed = 0, QObject* parent = 0);

    static int standardCap();
    void sætCap(int maksBytes);
    int cap() const;

    void feedBytesForTest(const QByteArray& data);
    void eofForTest();

signals:
    void beskedKlar(const QByteArray& json);
    void linjeDroppet(const QByteArray& linje, const QString& årsag);
    void haleReady(const QByteArray& hale);

private slots:
    void læsFraEnhed();

private:
    void bearbejd(const QByteArray& data);
    void håndterLinje(const QByteArray& linje);
    static bool gyldigUtf8(const QByteArray& bytes);
    static QList<QByteArray> splitTopLevel(const QByteArray& linje);

    QIODevice* m_enhed;
    QByteArray m_buffer;
    int m_cap;
};

#endif
