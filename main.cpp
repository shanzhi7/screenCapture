#include "mainwindow.h"
#include "settingsservice.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 发布版配置命名空间。
    QCoreApplication::setOrganizationName(QStringLiteral("QingYing"));
    QCoreApplication::setApplicationName(QStringLiteral("LightShadowCapture"));

    // 托盘模式下，关闭主窗口不代表退出进程。
    app.setQuitOnLastWindowClosed(false);

    // 首次启动时迁移旧命名空间配置（幂等）。
    SettingsService::initializeAndMigrateLegacySettings();

    // 应用级窗口图标（标题栏/托盘默认图标）。
    app.setWindowIcon(QIcon(QStringLiteral(":/icon.ico")));

    MainWindow window;
    window.show();

    return app.exec();
}
