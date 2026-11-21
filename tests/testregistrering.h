#ifndef TESTREGISTRATION_H
#define TESTREGISTRATION_H

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
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

inline void opretTestSettingsSti(const QString& navn)
{
    QStandardPaths::setTestModeEnabled(true);
    QString path = QDir::tempPath() + (navn.startsWith('/') ? navn : "/" + navn);
    QDir().mkpath(path);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, path);
    QSettings s;
    s.clear();
    s.sync();
}

inline QString testRepoRoot()
{
    QString root = QCoreApplication::applicationDirPath() + "/..";
    if (!QFileInfo::exists(root + "/realiseringsmanifest_for_funktioner_og_parrede_tests.json"))
        root = "..";
    return root;
}

inline QString testReadSource(const QString& rel)
{
    QFile f(testRepoRoot() + "/" + rel);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromUtf8(f.readAll());
}

inline QByteArray testReadFixture(const QString& name)
{
    QString path = QFINDTESTDATA("fixtures/" + name);
    if (path.isEmpty())
        path = "fixtures/" + name;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return f.readAll();
}

inline QJsonObject testJsonObj(const QByteArray& json)
{
    return QJsonDocument::fromJson(json).object();
}

inline qint64 testIdOf(const QByteArray& json)
{
    return testJsonObj(json).value("id").toVariant().toLongLong();
}

inline QByteArray testResponseJson(qint64 id, const QJsonObject& result)
{
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("result", result);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

inline QByteArray testErrorResponseJson(qint64 id, int kode, const QString& message)
{
    QJsonObject error;
    error.insert("code", kode);
    error.insert("message", message);
    QJsonObject o;
    o.insert("jsonrpc", "2.0");
    o.insert("id", QJsonValue(id));
    o.insert("error", error);
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

#endif
