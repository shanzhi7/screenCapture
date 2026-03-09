/***********************************************************************************
*
* @file         mainwindow.h
* @brief        主窗口：负责截图流程编排、历史展示与系统托盘交互。
*
* @author       shanzhi
* @date         2026/03/07
* @history
***********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef SCREENCAPTURE_ENABLE_LONG_CAPTURE
#define SCREENCAPTURE_ENABLE_LONG_CAPTURE 0
#endif

#include <QKeySequence>
#include <QMainWindow>
#include <QPixmap>
#include <QRect>
#include <QSystemTrayIcon>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class QAction;
class CaptureHistoryManager;
class CaptureResultHandler;
class CaptureUiStateCoordinator;
class GlobalHotkeyManager;
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
class LongCaptureSessionController;
#endif
class QCloseEvent;
class QGridLayout;
class QMenu;
class QResizeEvent;
class QShortcut;
class QTimer;
class SelectionOverlay;
class TipPresenter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#endif

private slots:
    // 启动当前模式截图流程。
    void startCapture();

    // 保存当前预览图到本地文件。
    void saveCurrentImage();

    // 普通区域截图确认完成。
    void onSelectionFinished(const QRect &rect);

    // 区域截图被取消。
    void onSelectionCanceled();

    // 覆盖层请求保存当前区域。
    void onOverlaySaveRequested(const QRect &rect);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    // 覆盖层切换长截图模式。
    void onOverlayLongCaptureToggled(bool enabled, const QRect &rect);

    // 长截图模式下的手动滚轮请求。
    void onOverlayLongCaptureWheel(const QRect &rect, int delta);

    // 长截图模式下请求保存。
    void onOverlayLongCaptureSaveRequested(const QRect &rect);

    // 长截图模式下请求确认复制。
    void onOverlayLongCaptureConfirmRequested(const QRect &rect);

    // 长截图控制器：预览更新。
    void onLongCapturePreviewUpdated(const QPixmap &pixmap);

    // 长截图控制器：可视高度更新。
    void onLongCaptureVisualHeightChanged(int height);

    // 长截图控制器：状态文案更新。
    void onLongCaptureStatusTextChanged(const QString &text);

    // 长截图控制器：复制结果就绪。
    void onLongCaptureCopyReady(const QPixmap &pixmap);

    // 长截图控制器：保存结果就绪。
    void onLongCaptureSaveReady(const QPixmap &pixmap);

    // 长截图控制器：失败回调。
    void onLongCaptureFailed(const QString &message);
#endif

    // 切换到全屏模式。
    void onModeFullClicked();

    // 切换到区域模式。
    void onModeRegionClicked();

    // 未实现按钮统一占位回调。
    void onPlaceholderAction();

    // 打开截图设置对话框。
    void onOpenSettingsRequested();

    // 打开关于/反馈对话框。
    void onOpenAboutRequested();

    // 托盘：显示主界面。
    void onTrayShowRequested();

    // 托盘：退出应用。
    void onTrayExitRequested();

    // 托盘图标激活回调。
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    enum class CaptureMode
    {
        Full,
        Region
    };

    enum class OutputFormat
    {
        PNG,
        JPG
    };

    // 初始化最近截图区域。
    void setupRecentItems();

    // 根据历史数据重绘最近截图卡片。
    void reloadRecentItems();

    // 清空最近截图网格中的所有卡片。
    void clearRecentItems();

    // 添加一个最近截图卡片。
    void addRecentItem(const QString &title,
                       const QString &timeText,
                       const QPixmap &thumbnail,
                       const QString &filePath,
                       int row,
                       int col);

    // 把新截图写入历史并刷新最近截图区域。
    void appendCaptureToHistory(const QPixmap &pixmap, const QString &title);

    // 从本地设置读取输出格式。
    void loadOutputFormat();

    // 把当前输出格式写入本地设置。
    void saveOutputFormat() const;

    // 刷新格式按钮文案。
    void updateFormatButtonText();

    // 切换截图输出格式。
    void toggleOutputFormat();

    // 返回当前输出格式扩展名（不带点）。
    QString outputFormatExtension() const;

    // 返回当前输出格式名称（用于 QPixmap::save）。
    QString outputFormatName() const;

    // 从本地设置读取自动保存开关。
    void loadAutoSaveEnabled();

    // 把自动保存开关写入本地设置。
    void saveAutoSaveEnabled() const;

    // 刷新自动保存按钮文案与状态。
    void updateAutoSaveButtonText();

    // 从本地设置读取自动保存目录。
    void loadAutoSaveDirectory();

    // 把自动保存目录写入本地设置。
    void saveAutoSaveDirectory() const;

    // 返回默认自动保存目录。
    QString defaultAutoSaveDirectory() const;

    // 返回当前生效自动保存目录（含用户配置与默认回退）。
    QString effectiveAutoSaveDirectory() const;

    // 按规则自动保存截图，必要时回退默认目录。
    bool autoSaveCapture(const QPixmap &pixmap,
                         const QString &sceneTag,
                         QString *savedPath = nullptr,
                         bool *usedFallbackDir = nullptr) const;

    // 使用文件对话框保存当前预览图。
    bool saveCurrentImageWithDialog(bool useMainWindowAsParent);

    // 根据截图会话状态恢复主窗口。
    void restoreWindowIfNeeded();

    // 结束截图会话状态。
    void endCaptureSession();

    // 刷新模式按钮选中视觉态。
    void updateModeSegmentVisuals();

    // 按全局坐标抓取指定区域（支持跨屏）。
    QPixmap captureRegion(const QRect &rect) const;

    // 重置截图运行态。
    void resetLongCaptureState();

    // 初始化系统托盘。
    void ensureTrayIcon();

    // 销毁并清理覆盖层。
    void dismissOverlay();

    // 刷新右侧预览区。
    void updatePreview(const QPixmap &pixmap);

    // 显示短时提示。
    void showTip(const QString &text);

    // 从本地设置读取截图热键。
    void loadCaptureHotkey();

    // 把截图热键写入本地设置。
    void saveCaptureHotkey() const;

    // 应用截图热键到当前平台。
    bool applyCaptureHotkey(const QKeySequence &sequence, bool showFeedback);

    // 刷新热键按钮文案。
    void updateHotkeyButtonText();

private:
    Ui::MainWindow *ui;
    SelectionOverlay *m_overlay = nullptr;
    std::unique_ptr<TipPresenter> m_tipPresenter;
    std::unique_ptr<CaptureUiStateCoordinator> m_uiStateCoordinator;
    std::unique_ptr<CaptureResultHandler> m_captureResultHandler;
    std::unique_ptr<GlobalHotkeyManager> m_globalHotkeyManager;

    QPixmap m_currentPixmap;
    CaptureMode m_captureMode = CaptureMode::Region;
    OutputFormat m_outputFormat = OutputFormat::PNG;
    bool m_autoSaveEnabled = true;
    QString m_autoSaveDirectory;
    QKeySequence m_captureHotkey = QKeySequence(QStringLiteral("Ctrl+Shift+A"));
    QShortcut *m_appHotkeyShortcut = nullptr;
    std::unique_ptr<CaptureHistoryManager> m_captureHistoryManager;
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    std::unique_ptr<LongCaptureSessionController> m_longCaptureController;
#endif

    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QAction *m_actionTrayShow = nullptr;
    QAction *m_actionTrayExit = nullptr;
    bool m_forceQuit = false;

#ifdef Q_OS_WIN
    static constexpr int kGlobalHotkeyId = 0x5A21;
#endif
};

#endif // MAINWINDOW_H

