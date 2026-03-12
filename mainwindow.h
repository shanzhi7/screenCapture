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

#include "longcapturetypes.h"

#include <QKeySequence>
#include <QMainWindow>
#include <QPixmap>
#include <QPointer>
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
    void startCapture();
    void saveCurrentImage();
    void onSelectionFinished(const QRect &rect);
    void onSelectionCanceled();
    void onOverlayColorValueCopied(const QString &colorText);
    void onOverlaySaveRequested(const QRect &rect);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void onOverlayLongCaptureToggled(bool enabled, const QRect &rect);
    void onOverlayLongCaptureWheel(const QRect &rect, int delta);
    void onOverlayLongCaptureSaveRequested(const QRect &rect);
    void onOverlayLongCaptureConfirmRequested(const QRect &rect);
    void onLongCapturePreviewUpdated(const QPixmap &pixmap);
    void onLongCapturePredictedVisualHeightChanged(int height);
    void onLongCaptureCommittedVisualHeightChanged(int height);
    void onLongCaptureCaptureQualityChanged(CaptureQuality quality);
    void onLongCaptureStatusTextChanged(const QString &text);
    void onLongCaptureCopyReady(const QPixmap &pixmap);
    void onLongCaptureSaveReady(const QPixmap &pixmap);
    void onLongCaptureFailed(const QString &message);
#endif

    void onModeFullClicked();
    void onModeRegionClicked();
    void onPlaceholderAction();
    void onOpenSettingsRequested();
    void onOpenAboutRequested();
    void onTrayShowRequested();
    void onTrayExitRequested();
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

    void setupRecentItems();
    void reloadRecentItems();
    void clearRecentItems();
    void addRecentItem(const QString &title,
                       const QString &timeText,
                       const QPixmap &thumbnail,
                       const QString &filePath,
                       int row,
                       int col);
    void appendCaptureToHistory(const QPixmap &pixmap, const QString &title);
    void loadOutputFormat();
    void saveOutputFormat() const;
    void updateFormatButtonText();
    void toggleOutputFormat();
    QString outputFormatExtension() const;
    QString outputFormatName() const;
    void loadAutoSaveEnabled();
    void saveAutoSaveEnabled() const;
    void updateAutoSaveButtonText();
    void loadAutoSaveDirectory();
    void saveAutoSaveDirectory() const;
    QString defaultAutoSaveDirectory() const;
    QString effectiveAutoSaveDirectory() const;
    bool autoSaveCapture(const QPixmap &pixmap,
                         const QString &sceneTag,
                         QString *savedPath = nullptr,
                         bool *usedFallbackDir = nullptr) const;
    bool saveCurrentImageWithDialog(bool useMainWindowAsParent);
    void restoreWindowIfNeeded();
    void endCaptureSession();
    void updateModeSegmentVisuals();
    QPixmap captureRegion(const QRect &rect) const;
    void resetLongCaptureState();
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    void stopLongCaptureForClosingOverlay();
#endif
    void ensureTrayIcon();
    void dismissOverlay();
    bool copyPixmapToClipboard(const QPixmap &pixmap) const;
    void updatePreview(const QPixmap &pixmap);
    void showTip(const QString &text);
    void loadCaptureHotkey();
    void saveCaptureHotkey() const;
    bool applyCaptureHotkey(const QKeySequence &sequence, bool showFeedback);
    void updateHotkeyButtonText();

private:
    Ui::MainWindow *ui;
    QPointer<SelectionOverlay> m_overlay;
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
    bool m_overlayClosing = false;
    bool m_forceQuit = false;

#ifdef Q_OS_WIN
    static constexpr int kGlobalHotkeyId = 0x5A21;
#endif
};

#endif // MAINWINDOW_H
