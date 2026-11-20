#ifndef TESTREGISTRERING_H
#define TESTREGISTRERING_H

#include <QList>
#include <QObject>
#include <QString>

typedef QObject* (*TestFabrik)();

struct TestRegistrering {
    QString navn;
    TestFabrik fabrik;
};

inline QList<TestRegistrering>& testRegistreringer()
{
    static QList<TestRegistrering> registreringer;
    return registreringer;
}

struct TestRegistrator {
    TestRegistrator(const QString& navn, TestFabrik fabrik)
    {
        TestRegistrering r;
        r.navn = navn;
        r.fabrik = fabrik;
        testRegistreringer().append(r);
    }
};

#define REGISTRER_TEST(klasse) \
    static QObject* lav_##klasse() { return new klasse; } \
    static TestRegistrator registrator_##klasse(#klasse, &lav_##klasse);

#endif
