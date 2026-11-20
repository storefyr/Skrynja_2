#include "acpframereader.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>

int AcpFrameReader::standardCap()
{
    return 65536;
}

AcpFrameReader::AcpFrameReader(QIODevice* enhed, QObject* parent)
    : QObject(parent)
    , m_enhed(enhed)
    , m_cap(standardCap())
{
    if (m_enhed)
        connect(m_enhed, SIGNAL(readyRead()), this, SLOT(læsFraEnhed()));
}

void AcpFrameReader::sætCap(int maksBytes)
{
    m_cap = maksBytes;
}

int AcpFrameReader::cap() const
{
    return m_cap;
}

void AcpFrameReader::læsFraEnhed()
{
    if (!m_enhed)
        return;
    bearbejd(m_enhed->readAll());
    if (m_enhed->atEnd() && !m_buffer.isEmpty()) {
        emit haleReady(m_buffer);
        m_buffer.clear();
    }
}

void AcpFrameReader::feedBytesForTest(const QByteArray& data)
{
    bearbejd(data);
}

void AcpFrameReader::eofForTest()
{
    if (!m_buffer.isEmpty()) {
        emit haleReady(m_buffer);
        m_buffer.clear();
    }
}

void AcpFrameReader::bearbejd(const QByteArray& data)
{
    m_buffer.append(data);
    int pos;
    while ((pos = m_buffer.indexOf('\n')) >= 0) {
        QByteArray linje = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);
        if (linje.endsWith('\r'))
            linje.chop(1);
        håndterLinje(linje);
    }
}

void AcpFrameReader::håndterLinje(const QByteArray& linje)
{
    if (linje.isEmpty())
        return;
    if (linje.size() > m_cap) {
        emit linjeDroppet(linje, "cap");
        return;
    }
    if (!gyldigUtf8(linje)) {
        emit linjeDroppet(linje, "ugyldig utf-8");
        return;
    }
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(linje, &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        emit beskedKlar(linje);
        return;
    }
    QList<QByteArray> dele = splitTopLevel(linje);
    if (dele.size() > 1) {
        for (int i = 0; i < dele.size(); ++i) {
            QJsonParseError delErr;
            QJsonDocument delDoc = QJsonDocument::fromJson(dele.at(i), &delErr);
            if (delErr.error == QJsonParseError::NoError && delDoc.isObject())
                emit beskedKlar(dele.at(i));
            else
                emit linjeDroppet(dele.at(i), "ugyldig json");
        }
        return;
    }
    emit linjeDroppet(linje, "ugyldig json");
}

bool AcpFrameReader::gyldigUtf8(const QByteArray& bytes)
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

QList<QByteArray> AcpFrameReader::splitTopLevel(const QByteArray& linje)
{
    QList<QByteArray> dele;
    int i = 0;
    const int n = linje.size();
    while (i < n) {
        while (i < n && (linje.at(i) == ' ' || linje.at(i) == '\t'))
            ++i;
        if (i >= n)
            break;
        int start = i;
        if (linje.at(i) == '{' || linje.at(i) == '[') {
            char luk = (linje.at(i) == '{') ? '}' : ']';
            int depth = 0;
            bool iStreng = false;
            bool esc = false;
            for (; i < n; ++i) {
                char c = linje.at(i);
                if (iStreng) {
                    if (esc)
                        esc = false;
                    else if (c == '\\')
                        esc = true;
                    else if (c == '"')
                        iStreng = false;
                } else {
                    if (c == '"')
                        iStreng = true;
                    else if (c == linje.at(start))
                        ++depth;
                    else if (c == luk) {
                        --depth;
                        if (depth == 0) {
                            ++i;
                            break;
                        }
                    }
                }
            }
            dele.append(linje.mid(start, i - start));
        } else {
            dele.append(linje.mid(start));
            i = n;
        }
    }
    return dele;
}
