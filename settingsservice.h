/***********************************************************************************
*
* @file         settingsservice.h
* @brief        配置服务：统一管理 QSettings 读写与旧配置迁移。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QString>

class SettingsService
{
public:
    // 初始化并执行旧命名空间配置迁移（幂等）。
    static void initializeAndMigrateLegacySettings();

    // 读取布尔配置。
    static bool readBool(const QString &key, bool defaultValue);

    // 写入布尔配置。
    static void writeBool(const QString &key, bool value);

    // 读取字符串配置。
    static QString readString(const QString &key, const QString &defaultValue);

    // 写入字符串配置。
    static void writeString(const QString &key, const QString &value);

private:
    static QString migratedFlagKey();
};

#endif // SETTINGSSERVICE_H
