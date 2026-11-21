#ifndef CONFIGURATIONMODEL_H
#define CONFIGURATIONMODEL_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class ConfigurationModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList configOptions READ configOptions NOTIFY changed)
public:
    explicit ConfigurationModel(QObject* parent = 0);

    void setConfigOptions(const QVariantList& options);
    QVariantList configOptions() const;
    QVariantMap findConfig(const QString& configId) const;

signals:
    void changed();

private:
    QVariantList m_options;
};

#endif
