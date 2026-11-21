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
    void setCap(int maksBytes);
    int cap() const;

signals:
    void messageReady(const QByteArray& json);
    void lineDropped(const QByteArray& line, const QString& reason);
    void haleReady(const QByteArray& hale);

protected slots:
    void readFromDevice();

protected:
    void processBuffer(const QByteArray& data);
    void flushHale();

private:
    void handleLine(const QByteArray& line);
    static bool isValidUtf8(const QByteArray& bytes);
    static QList<QByteArray> splitTopLevel(const QByteArray& line);

    QIODevice* m_device;
    QByteArray m_buffer;
    int m_cap;
};

#endif
