#include "settingsservice.h"

#include <QSettings>
#include <QStringList>

namespace
{
const char *kLegacyOrg = "shanzhi";
const char *kLegacyApp = "screenCapture";
}

void SettingsService::initializeAndMigrateLegacySettings()
{
    QSettings currentSettings;
    if (currentSettings.value(migratedFlagKey(), false).toBool())
    {
        return;
    }

    QSettings legacySettings(QString::fromUtf8(kLegacyOrg), QString::fromUtf8(kLegacyApp));
    const QStringList keys = legacySettings.allKeys();

    // 迁移策略：新命名空间已有值优先，不被旧值覆盖。
    for (const QString &key : keys)
    {
        if (currentSettings.contains(key))
        {
            continue;
        }

        currentSettings.setValue(key, legacySettings.value(key));
    }

    currentSettings.setValue(migratedFlagKey(), true);
    currentSettings.sync();
}

bool SettingsService::readBool(const QString &key, bool defaultValue)
{
    QSettings settings;
    return settings.value(key, defaultValue).toBool();
}

void SettingsService::writeBool(const QString &key, bool value)
{
    QSettings settings;
    settings.setValue(key, value);
}

QString SettingsService::readString(const QString &key, const QString &defaultValue)
{
    QSettings settings;
    return settings.value(key, defaultValue).toString();
}

void SettingsService::writeString(const QString &key, const QString &value)
{
    QSettings settings;
    settings.setValue(key, value);
}

QString SettingsService::migratedFlagKey()
{
    return QStringLiteral("meta/settings_migrated_from_legacy");
}
