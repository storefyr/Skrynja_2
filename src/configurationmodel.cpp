#include "configurationmodel.h"

ConfigurationModel::ConfigurationModel(QObject* parent)
    : QObject(parent)
{
}

void ConfigurationModel::setConfigOptions(const QVariantList& options)
{
    m_options = options;
    emit changed();
}

QVariantList ConfigurationModel::configOptions() const
{
    return m_options;
}

QVariantMap ConfigurationModel::findConfig(const QString& configId) const
{
    for (int i = 0; i < m_options.size(); ++i) {
        QVariantMap m = m_options.at(i).toMap();
        if (m.value("configId").toString() == configId)
            return m;
    }
    return QVariantMap();
}
