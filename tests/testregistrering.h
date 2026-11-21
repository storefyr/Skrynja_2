#ifndef TESTREGISTRATION_H
#define TESTREGISTRATION_H

#include <QList>
#include <QObject>
#include <QString>

typedef QObject* (*TestFabrik)();

struct TestRegistration {
    QString name;
    TestFabrik factory;
};

inline QList<TestRegistration>& testRegistrations()
{
    static QList<TestRegistration> registreringer;
    return registreringer;
}

struct TestRegistrar {
    TestRegistrar(const QString& name, TestFabrik factory)
    {
        TestRegistration r;
        r.name = name;
        r.factory = factory;
        testRegistrations().append(r);
    }
};

#define REGISTER_TEST(klasse) \
    static QObject* lav_##klasse() { return new klasse; } \
    static TestRegistrar registrator_##klasse(#klasse, &lav_##klasse);

#endif
