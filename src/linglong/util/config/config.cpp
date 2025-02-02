/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"

#include "linglong/package/ref.h"
#include "linglong/util/file.h"
#include "linglong/util/qserializer/yaml.h"

namespace linglong::util::config {

QSERIALIZER_IMPL(Config)

static const char *const kConfigFileName = "config.yaml";

QSharedPointer<Config> loadConfig()
{
    auto configFilePath = findLinglongConfigPath(kConfigFileName, false);
    qDebug() << "load" << configFilePath;
    QSharedPointer<Config> config;
    try {
        config = std::get<0>(fromYAML<QSharedPointer<Config>>(configFilePath));
    } catch (...) {
        qWarning() << "Failed to load config, cfg:" << config;
    }

    if (!config) {
        qWarning() << "load config failed";
        config = QSharedPointer<Config>(new Config());
    }

    Q_ASSERT(config);

    if (!config->repos.contains(package::kDefaultRepo)) {
        qWarning() << "load config for" << package::kDefaultRepo << "failed";
        config->repos.insert(package::kDefaultRepo, QSharedPointer<Repo>(new Repo(config.data())));
    }

    Q_ASSERT(config->repos.contains(package::kDefaultRepo));

    config->self = config;
    config->path = findLinglongConfigPath(kConfigFileName, true);
    return config;
}

/*!
 * save to a writable path, like /var/lib/linglong
 */
void Config::save()
{
    auto [data, err] = toYAML(this->self.toStrongRef());
    if (err) {
        qCritical() << "convert to yaml failed with" << err;
        return;
    }

    QFile configFile(this->path);
    configFile.open(QIODevice::WriteOnly);
    if (configFile.error()) {
        qCritical() << "save to" << this->path << "failed:" << configFile.errorString();
        return;
    }
    configFile.write(data);
    configFile.close();
}

Config &ConfigInstance()
{
    static QSharedPointer<Config> config(loadConfig());
    return *config;
}

} // namespace linglong::util::config
