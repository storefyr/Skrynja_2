#ifndef TESTREGISTRATION_H
#define TESTREGISTRATION_H

#include <QList>
#include <QObject>
#include <QString>

#include "acpclient.h"
#include "konfigurationsmodel.h"
#include "diffmodel.h"
#include "samtalemodel.h"
#include "tilladelsesmodel.h"
#include "vaerktoejskaldsmodel.h"

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

inline void injectTestModels(AcpClient* client, QObject* ejer)
{
    client->setConversationModel(new ConversationModel(ejer));
    client->setToolCallModel(new ToolCallModel(ejer));
    client->setPermissionModel(new PermissionModel(ejer));
    client->setDiffModel(new DiffModel(ejer));
    client->setConfigurationModel(new ConfigurationModel(ejer));
}

#endif
