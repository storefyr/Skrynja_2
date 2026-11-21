#include "acpframereader.h"

#include "acpprotokol.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>

int AcpFrameReader::standardCap()
{
    return AcpProtokol::standardCap();
}

AcpFrameReader::AcpFrameReader(QIODevice* enhed, QObject* parent)
    : QObject(parent)
    , m_device(enhed)
    , m_cap(standardCap())
{
    if (m_device)
        connect(m_device, SIGNAL(readyRead()), this, SLOT(readFromDevice()));
}

void AcpFrameReader::setCap(int maksBytes)
{
    m_cap = maksBytes;
}

int AcpFrameReader::cap() const
{
    return m_cap;
}

void AcpFrameReader::readFromDevice()
{
    if (!m_device)
        return;
    processBuffer(m_device->readAll());
    if (m_device->atEnd())
        flushHale();
}

void AcpFrameReader::flushHale()
{
    if (!m_buffer.isEmpty()) {
        emit haleReady(m_buffer);
        m_buffer.clear();
    }
}

void AcpFrameReader::processBuffer(const QByteArray& data)
{
    m_buffer.append(data);
    int pos;
    while ((pos = m_buffer.indexOf('\n')) >= 0) {
        QByteArray line = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);
        if (line.endsWith('\r'))
            line.chop(1);
        handleLine(line);
    }
}

void AcpFrameReader::handleLine(const QByteArray& line)
{
    if (line.isEmpty())
        return;
    if (line.size() > m_cap) {
        emit lineDropped(line, "cap");
        return;
    }
    if (!isValidUtf8(line)) {
        emit lineDropped(line, "ugyldig utf-8");
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(line, &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        emit messageReady(line);
        return;
    }
    QList<QByteArray> parts = splitTopLevel(line);
    if (parts.size() > 1) {
        for (int i = 0; i < parts.size(); ++i) {
            QJsonParseError delErr;
            QJsonDocument delDoc = QJsonDocument::fromJson(parts.at(i), &delErr);
            if (delErr.error == QJsonParseError::NoError && delDoc.isObject())
                emit messageReady(parts.at(i));
            else
                emit lineDropped(parts.at(i), "ugyldig json");
        }
        return;
    }
    emit lineDropped(line, "ugyldig json");
}

bool AcpFrameReader::isValidUtf8(const QByteArray& bytes)
{
    int i = 0;
    const int n = bytes.size();
    while (i < n) {
        unsigned char c = (unsigned char)bytes.at(i);
        if (c < 0x80) {
            ++i;
            continue;
        }
        int ekstra;
        unsigned int min;
        if ((c & 0xE0) == 0xC0) {
            ekstra = 1;
            min = 0x80;
        } else if ((c & 0xF0) == 0xE0) {
            ekstra = 2;
            min = 0x800;
        } else if ((c & 0xF8) == 0xF0) {
            ekstra = 3;
            min = 0x10000;
        } else {
            return false;
        }
        if (i + ekstra >= n)
            return false;
        unsigned int cp = c & (0x3F >> ekstra);
        for (int k = 1; k <= ekstra; ++k) {
            unsigned char cc = (unsigned char)bytes.at(i + k);
            if ((cc & 0xC0) != 0x80)
                return false;
            cp = (cp << 6) | (cc & 0x3F);
        }
        if (cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return false;
        i += ekstra + 1;
    }
    return true;
}

QList<QByteArray> AcpFrameReader::splitTopLevel(const QByteArray& line)
{
    QList<QByteArray> parts;
    int i = 0;
    const int n = line.size();
    while (i < n) {
        while (i < n && (line.at(i) == ' ' || line.at(i) == '\t'))
            ++i;
        if (i >= n)
            break;
        int start = i;
        if (line.at(i) == '{' || line.at(i) == '[') {
            char closeBrace = (line.at(i) == '{') ? '}' : ']';
            int depth = 0;
            bool inString = false;
            bool esc = false;
            for (; i < n; ++i) {
                char c = line.at(i);
                if (inString) {
                    if (esc)
                        esc = false;
                    else if (c == '\\')
                        esc = true;
                    else if (c == '"')
                        inString = false;
                } else {
                    if (c == '"')
                        inString = true;
                    else if (c == line.at(start))
                        ++depth;
                    else if (c == closeBrace) {
                        --depth;
                        if (depth == 0) {
                            ++i;
                            break;
                        }
                    }
                }
            }
            parts.append(line.mid(start, i - start));
        } else {
            parts.append(line.mid(start));
            i = n;
        }
    }
    return parts;
}
