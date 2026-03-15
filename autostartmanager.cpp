#include "autostartmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

namespace
{
QString runRegistryPath()
{
    return QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
}

QString startupValueName()
{
    const QString appName = QCoreApplication::applicationName().trimmed();
    return appName.isEmpty() ? QStringLiteral("screenCapture") : appName;
}

QString startupCommand()
{
    const QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    return appPath.isEmpty() ? QString() : QStringLiteral("\"%1\"").arg(appPath);
}
}

bool AutoStartManager::isSupported()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

bool AutoStartManager::isEnabled()
{
#ifdef Q_OS_WIN
    QSettings runSettings(runRegistryPath(), QSettings::NativeFormat);
    return !runSettings.value(startupValueName()).toString().trimmed().isEmpty();
#else
    return false;
#endif
}

bool AutoStartManager::setEnabled(bool enabled, QString *errorMessage)
{
#ifdef Q_OS_WIN
    if (errorMessage != nullptr)
    {
        errorMessage->clear();
    }

    QSettings runSettings(runRegistryPath(), QSettings::NativeFormat);
    const QString valueName = startupValueName();

    if (enabled)
    {
        const QString command = startupCommand();
        if (command.isEmpty())
        {
            if (errorMessage != nullptr)
            {
                *errorMessage = QStringLiteral("无法解析程序路径");
            }
            return false;
        }

        runSettings.setValue(valueName, command);
    }
    else
    {
        runSettings.remove(valueName);
    }

    runSettings.sync();
    if (runSettings.status() != QSettings::NoError)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = QStringLiteral("写入开机自启动配置失败");
        }
        return false;
    }

    const bool actualEnabled = isEnabled();
    if (actualEnabled != enabled)
    {
        if (errorMessage != nullptr)
        {
            *errorMessage = enabled
                                ? QStringLiteral("启用开机自启动失败")
                                : QStringLiteral("关闭开机自启动失败");
        }
        return false;
    }

    return true;
#else
    if (enabled && errorMessage != nullptr)
    {
        *errorMessage = QStringLiteral("当前平台暂不支持开机自启动");
    }
    return !enabled;
#endif
}