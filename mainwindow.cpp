#include "mainwindow.h"

#include "aboutdialog.h"
#include "autostartmanager.h"
#include "capturehistorymanager.h"
#include "capturesettingsdialog.h"
#include "captureresulthandler.h"
#include "draggablehistorythumbbutton.h"
#include "captureuistatecoordinator.h"
#include "globalhotkeymanager.h"
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
#include "longcapturesessioncontroller.h"
#endif
#include "pinnedimagewindow.h"
#include "selectionoverlay.h"
#include "settingsservice.h"
#include "tippresenter.h"
#include "showtip.h"
#include "ui_mainwindow.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QCursor>
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QShortcut>
#include <QStringList>
#include <QSize>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
constexpr int kRecentHistoryPreviewCount = 8;
constexpr int kHistoryGridColumnCount = 4;
const QSize kHistoryThumbnailSize(220, 120);
}
#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tipPresenter(std::make_unique<TipPresenter>())
    , m_uiStateCoordinator(std::make_unique<CaptureUiStateCoordinator>())
    , m_captureResultHandler(std::make_unique<CaptureResultHandler>())
    , m_captureHistoryManager(std::make_unique<CaptureHistoryManager>())
{
    ui->setupUi(this);

    QFile styleFile(QStringLiteral(":/styles/hyperos.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }

    auto applyIcon = [](QAbstractButton *button,
                        const QString &iconPath,
                        const QSize &iconSize,
                        const QString &fallbackText)
    {
        QIcon icon(iconPath);
        if (icon.isNull() && iconPath.startsWith(QStringLiteral(":/icons/")))
        {
            const QString fallback = QStringLiteral(":/icons/icons/") + iconPath.mid(QStringLiteral(":/icons/").size());
            icon = QIcon(fallback);
        }

        if (icon.isNull())
        {
            button->setText(fallbackText);
            return;
        }

        button->setText(QString());
        button->setIcon(icon);

        if (auto tb = qobject_cast<QToolButton *>(button))
        {
            tb->setIconSize(iconSize);
        }
        else if (auto pb = qobject_cast<QPushButton *>(button))
        {
            pb->setIconSize(iconSize);
        }
    };

    applyIcon(ui->sideCaptureButton, QStringLiteral(":/icons/camera.svg"), QSize(24, 24), QStringLiteral("拍"));
    applyIcon(ui->sideGalleryButton, QStringLiteral(":/icons/folder.svg"), QSize(24, 24), QStringLiteral("存"));
    ui->sideGalleryButton->setToolTip(QStringLiteral("打开自动保存目录"));
    applyIcon(ui->sideConfigButton, QStringLiteral(":/icons/settings.svg"), QSize(24, 24), QStringLiteral("设"));
    applyIcon(ui->sideBottomButton, QStringLiteral(":/icons/info.svg"), QSize(24, 24), QStringLiteral("关"));
    applyIcon(ui->btnPrev, QStringLiteral(":/icons/chevron_left.svg"), QSize(20, 20), QStringLiteral("<"));
    applyIcon(ui->btnShotIcon, QStringLiteral(":/icons/crop.svg"), QSize(20, 20), QStringLiteral("裁"));
    applyIcon(ui->btnNext, QStringLiteral(":/icons/chevron_right.svg"), QSize(20, 20), QStringLiteral(">"));
    applyIcon(ui->btnTopSettings, QStringLiteral(":/icons/settings.svg"), QSize(18, 18), QStringLiteral("设"));

    ui->modeFullButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->modeRegionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->modeLayout->setContentsMargins(4, 4, 4, 4);
    ui->modeLayout->setSpacing(6);
    ui->modeLayout->setStretch(0, 1);
    ui->modeLayout->setStretch(1, 1);
    ui->modeScrollButton->hide();

    loadOutputFormat();
    updateFormatButtonText();

    loadAutoSaveEnabled();
    loadAutoSaveDirectory();
    updateAutoSaveButtonText();

    loadCaptureHotkey();

    connect(ui->startCaptureButton, &QPushButton::clicked, this, &MainWindow::startCapture);
    connect(ui->modeFullButton, &QToolButton::clicked, this, &MainWindow::onModeFullClicked);
    connect(ui->modeRegionButton, &QToolButton::clicked, this, &MainWindow::onModeRegionClicked);

#ifndef Q_OS_WIN
    m_appHotkeyShortcut = new QShortcut(this);
    m_appHotkeyShortcut->setContext(Qt::ApplicationShortcut);
    connect(m_appHotkeyShortcut, &QShortcut::activated, this, &MainWindow::startCapture);
#endif

#ifdef Q_OS_WIN
    m_globalHotkeyManager = std::make_unique<GlobalHotkeyManager>(this, kGlobalHotkeyId, this);
#else
    m_globalHotkeyManager = std::make_unique<GlobalHotkeyManager>(this, 0, this);
#endif
    connect(m_globalHotkeyManager.get(), &GlobalHotkeyManager::activated, this, &MainWindow::startCapture);
    applyCaptureHotkey(m_captureHotkey, false);
    updateHotkeyButtonText();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_longCaptureController = std::make_unique<LongCaptureSessionController>(this);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::previewUpdated, this, &MainWindow::onLongCapturePreviewUpdated);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::predictedVisualHeightChanged, this, &MainWindow::onLongCapturePredictedVisualHeightChanged);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::committedVisualHeightChanged, this, &MainWindow::onLongCaptureCommittedVisualHeightChanged);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::captureQualityChanged, this, &MainWindow::onLongCaptureCaptureQualityChanged);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::statusTextChanged, this, &MainWindow::onLongCaptureStatusTextChanged);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::copyReady, this, &MainWindow::onLongCaptureCopyReady);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::saveReady, this, &MainWindow::onLongCaptureSaveReady);
    connect(m_longCaptureController.get(), &LongCaptureSessionController::failed, this, &MainWindow::onLongCaptureFailed);
#endif

    connect(ui->btnTopSettings, &QAbstractButton::clicked, this, &MainWindow::onOpenSettingsRequested);
    connect(ui->sideConfigButton, &QAbstractButton::clicked, this, &MainWindow::onOpenSettingsRequested);
    connect(ui->btnHotkeySetting, &QAbstractButton::clicked, this, &MainWindow::onOpenSettingsRequested);
    connect(ui->sideBottomButton, &QAbstractButton::clicked, this, &MainWindow::onOpenAboutRequested);
    connect(ui->sideCaptureButton, &QAbstractButton::clicked, this, &MainWindow::startCapture);
    connect(ui->sideGalleryButton, &QAbstractButton::clicked, this, &MainWindow::onOpenAutoSaveDirectoryRequested);
    connect(ui->btnMoreRecent, &QAbstractButton::clicked, this, &MainWindow::onMoreRecentRequested);
    connect(ui->btnPrev, &QAbstractButton::clicked, this, &MainWindow::onHistoryPreviousRequested);
    connect(ui->btnShotIcon, &QAbstractButton::clicked, this, &MainWindow::onHistoryLatestRequested);
    connect(ui->btnNext, &QAbstractButton::clicked, this, &MainWindow::onHistoryNextRequested);

    const QList<QAbstractButton *> placeholderButtons = {
        ui->btnMoreFormat,
        ui->btnFormatSetting,
        ui->btnAutoSave
    };

    for (QAbstractButton *button : placeholderButtons)
    {
        connect(button, &QAbstractButton::clicked, this, &MainWindow::onPlaceholderAction);
    }
    ensureTrayIcon();
    setupRecentItems();
    updateModeSegmentVisuals();
    showTip(QStringLiteral("应用已启动"));
}

MainWindow::~MainWindow()
{
    if (m_trayIcon != nullptr)
    {
        m_trayIcon->hide();
    }

    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!m_currentPixmap.isNull())
    {
        updatePreview(m_currentPixmap);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_forceQuit)
    {
        if (m_trayIcon != nullptr)
        {
            m_trayIcon->hide();
        }
        event->accept();
        return;
    }

    ensureTrayIcon();
    const ShowTip::CloseChoice choice = ShowTip::askCloseChoice(this, QStringLiteral("轻影截图"));

    if (choice == ShowTip::CloseChoice::HideToTray)
    {
        event->ignore();
        hide();

        if (m_uiStateCoordinator != nullptr)
        {
            m_uiStateCoordinator->markHiddenToTray(true);
        }

        if (m_trayIcon != nullptr)
        {
            m_trayIcon->showMessage(QStringLiteral("轻影截图"),
                                    QStringLiteral("应用已隐藏到系统托盘，仍可使用截图快捷键。"),
                                    QSystemTrayIcon::Information,
                                    1800);
        }
        return;
    }

    if (choice == ShowTip::CloseChoice::ExitApp)
    {
        m_forceQuit = true;
        if (m_trayIcon != nullptr)
        {
            m_trayIcon->hide();
        }
        event->ignore();
        qApp->quit();
        return;
    }

    event->ignore();
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType)

    MSG *msg = static_cast<MSG *>(message);
    if (m_globalHotkeyManager != nullptr && m_globalHotkeyManager->handleNativeEvent(msg, result))
    {
        return true;
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::startCapture()
{
    if (m_overlay != nullptr && m_overlay->isVisible())
    {
        showTip(QStringLiteral("截图进行中"));
        return;
    }

    resetLongCaptureState();
    m_overlayClosing = false;

    if (m_uiStateCoordinator != nullptr)
    {
        const bool keepHidden = m_uiStateCoordinator->isHiddenToTray() || !isVisible();
        m_uiStateCoordinator->beginCaptureSession(keepHidden);
    }

    if (m_captureMode == CaptureMode::Full)
    {
        const QList<QScreen *> screens = QGuiApplication::screens();
        if (screens.isEmpty())
        {
            showTip(QStringLiteral("未检测到可用显示器"));
            endCaptureSession();
            return;
        }

        QScreen *targetScreen = QGuiApplication::screenAt(QCursor::pos());
        if (targetScreen == nullptr)
        {
            targetScreen = screens.first();
        }

        if (screens.size() > 1)
        {
            QStringList items;
            int defaultIndex = 0;

            for (int i = 0; i < screens.size(); ++i)
            {
                QScreen *screen = screens.at(i);
                const QRect g = screen->geometry();
                const QString name = screen->name().isEmpty() ? QStringLiteral("显示器 %1").arg(i + 1) : screen->name();
                items << QStringLiteral("%1 (%2,%3 %4x%5)")
                             .arg(name)
                             .arg(g.x())
                             .arg(g.y())
                             .arg(g.width())
                             .arg(g.height());

                if (screen == targetScreen)
                {
                    defaultIndex = i;
                }
            }

            bool ok = false;
            QWidget *dialogParent = (m_uiStateCoordinator != nullptr && (m_uiStateCoordinator->isHiddenToTray() || !isVisible()))
                                        ? nullptr
                                        : static_cast<QWidget *>(this);

            const QString selected = QInputDialog::getItem(dialogParent,
                                                           QStringLiteral("选择显示器"),
                                                           QStringLiteral("请选择目标显示器："),
                                                           items,
                                                           defaultIndex,
                                                           false,
                                                           &ok);
            if (!ok || selected.isEmpty())
            {
                showTip(QStringLiteral("截图已取消"));
                endCaptureSession();
                return;
            }

            const int chosenIndex = items.indexOf(selected);
            if (chosenIndex >= 0 && chosenIndex < screens.size())
            {
                targetScreen = screens.at(chosenIndex);
            }
        }

        hide();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);

        ShowTip::setCaptureSuppressed(true);
        const QPixmap pixmap = targetScreen->grabWindow(0);
        ShowTip::setCaptureSuppressed(false);

        const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                    ? true
                                                    : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
        const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                                   ? CaptureResultDecision{shouldRestoreWindowByState, false}
                                                   : m_captureResultHandler->decide(CaptureResultAction::FullScreenCaptured,
                                                                                    shouldRestoreWindowByState,
                                                                                    m_autoSaveEnabled);

        if (decision.shouldRestoreWindow)
        {
            restoreWindowIfNeeded();
        }

        if (pixmap.isNull())
        {
            showTip(QStringLiteral("全屏截图失败"));
            endCaptureSession();
            return;
        }

        updatePreview(pixmap);
        copyPixmapToClipboard(pixmap);
        appendCaptureToHistory(pixmap, QStringLiteral("全屏截图"));

        if (m_autoSaveEnabled)
        {
            QString savedPath;
            bool usedFallbackDir = false;
            if (autoSaveCapture(pixmap, QStringLiteral("full"), &savedPath, &usedFallbackDir))
            {
                if (usedFallbackDir)
                {
                    showTip(QStringLiteral("截图成功，已回退默认目录保存：%1")
                                .arg(QFileInfo(savedPath).fileName()));
                }
                else
                {
                    showTip(QStringLiteral("截图成功，已自动保存：%1")
                                .arg(QFileInfo(savedPath).fileName()));
                }
            }
            else
            {
                showTip(QStringLiteral("截图成功，但自动保存失败"));
            }
        }
        else
        {
            showTip(QStringLiteral("截图已复制到剪贴板"));
        }

        endCaptureSession();
        return;
    }

    m_overlay = new SelectionOverlay();
    connect(m_overlay, &SelectionOverlay::selectionFinished, this, &MainWindow::onSelectionFinished);
    connect(m_overlay, &SelectionOverlay::selectionCanceled, this, &MainWindow::onSelectionCanceled);
    connect(m_overlay, &SelectionOverlay::colorValueCopied, this, &MainWindow::onOverlayColorValueCopied);
    connect(m_overlay, &SelectionOverlay::saveRequested, this, &MainWindow::onOverlaySaveRequested);
    connect(m_overlay, &SelectionOverlay::pinRequested, this, &MainWindow::onOverlayPinRequested);
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    connect(m_overlay, &SelectionOverlay::longCaptureToggled, this, &MainWindow::onOverlayLongCaptureToggled);
    connect(m_overlay, &SelectionOverlay::longCaptureWheel, this, &MainWindow::onOverlayLongCaptureWheel);
    connect(m_overlay, &SelectionOverlay::longCaptureSaveRequested, this, &MainWindow::onOverlayLongCaptureSaveRequested);
    connect(m_overlay, &SelectionOverlay::longCaptureConfirmRequested, this, &MainWindow::onOverlayLongCaptureConfirmRequested);
#endif

    connect(m_overlay, &QObject::destroyed, this, [this]()
    {
        m_overlay = nullptr;
        m_overlayClosing = false;
    });

    hide();
    m_overlay->show();
    m_overlay->activateWindow();
    m_overlay->setFocus();
}
void MainWindow::saveCurrentImage()
{
    const bool useMainWindowAsParent = (m_uiStateCoordinator == nullptr)
                                           ? true
                                           : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
    saveCurrentImageWithDialog(useMainWindowAsParent);
}

bool MainWindow::saveCurrentImageWithDialog(bool useMainWindowAsParent)
{
    if (m_currentPixmap.isNull())
    {
        showTip(QStringLiteral("当前没有可保存的图片"));
        return false;
    }

    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (defaultDir.isEmpty())
    {
        defaultDir = QDir::homePath();
    }

    const QString defaultName = QStringLiteral("截图_%1.%2")
                                    .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")))
                                    .arg(outputFormatExtension());

    const QString defaultPath = QDir(defaultDir).filePath(defaultName);
    const QString filter = (m_outputFormat == OutputFormat::PNG)
                               ? QStringLiteral("PNG (*.png)")
                               : QStringLiteral("JPG (*.jpg *.jpeg)");

    QWidget *dialogParent = useMainWindowAsParent ? static_cast<QWidget *>(this) : nullptr;
    QString path = QFileDialog::getSaveFileName(dialogParent,
                                                QStringLiteral("保存截图"),
                                                defaultPath,
                                                filter);
    if (path.isEmpty())
    {
        return false;
    }

    if (QFileInfo(path).suffix().isEmpty())
    {
        path += QStringLiteral(".") + outputFormatExtension();
    }

    const int quality = (m_outputFormat == OutputFormat::JPG) ? 95 : -1;
    if (m_currentPixmap.save(path, outputFormatName().toUtf8().constData(), quality))
    {
        showTip(QStringLiteral("保存成功"));
        return true;
    }

    showTip(QStringLiteral("保存失败"));
    return false;
}
void MainWindow::onSelectionFinished(const QRect &rect)
{
    QTimer::singleShot(90, this, [this, rect]()
    {
        const QPixmap pixmap = buildSelectionResultPixmap(rect);

        const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                    ? true
                                                    : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
        const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                                   ? CaptureResultDecision{shouldRestoreWindowByState, false}
                                                   : m_captureResultHandler->decide(CaptureResultAction::RegionConfirmed,
                                                                                    shouldRestoreWindowByState,
                                                                                    m_autoSaveEnabled);

        if (decision.shouldRestoreWindow)
        {
            restoreWindowIfNeeded();
        }

        if (pixmap.isNull())
        {
            dismissOverlay();
            showTip(QStringLiteral("区域截图失败"));
            endCaptureSession();
            return;
        }

        applyRegionResult(pixmap, QStringLiteral("区域截图"), true);
        dismissOverlay();
        showTip(QStringLiteral("截图已复制到剪贴板"));
        endCaptureSession();
    });
}

void MainWindow::onSelectionCanceled()
{
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    stopLongCaptureForClosingOverlay();
#endif

    const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                ? true
                                                : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
    const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                               ? CaptureResultDecision{shouldRestoreWindowByState, false}
                                               : m_captureResultHandler->decide(CaptureResultAction::SelectionCanceled,
                                                                                shouldRestoreWindowByState,
                                                                                m_autoSaveEnabled);

    if (decision.shouldRestoreWindow)
    {
        restoreWindowIfNeeded();
    }

    showTip(QStringLiteral("截图已取消"));
    endCaptureSession();
}

void MainWindow::onOverlayColorValueCopied(const QString &colorText)
{
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    stopLongCaptureForClosingOverlay();
#else
    resetLongCaptureState();
#endif

    const bool shouldRestoreWindow = (m_uiStateCoordinator == nullptr)
                                         ? true
                                         : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
    if (shouldRestoreWindow)
    {
        restoreWindowIfNeeded();
    }

    endCaptureSession();
    showTip(colorText.isEmpty()
                ? QStringLiteral("已复制色值")
                : QStringLiteral("已复制色值：%1").arg(colorText));
}

void MainWindow::onOverlaySaveRequested(const QRect &rect)
{
    QTimer::singleShot(90, this, [this, rect]()
    {
        const QPixmap pixmap = buildSelectionResultPixmap(rect);

        const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                    ? true
                                                    : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
        const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                                   ? CaptureResultDecision{shouldRestoreWindowByState, !m_autoSaveEnabled}
                                                   : m_captureResultHandler->decide(CaptureResultAction::RegionSaveRequested,
                                                                                    shouldRestoreWindowByState,
                                                                                    m_autoSaveEnabled);

        if (decision.shouldRestoreWindow)
        {
            restoreWindowIfNeeded();
        }

        if (pixmap.isNull())
        {
            dismissOverlay();
            showTip(QStringLiteral("保存截图失败"));
            endCaptureSession();
            return;
        }

        applyRegionResult(pixmap, QStringLiteral("区域截图"), false);

        if (!decision.useManualSaveDialog)
        {
            QString savedPath;
            bool usedFallbackDir = false;
            if (autoSaveCapture(pixmap, QStringLiteral("region"), &savedPath, &usedFallbackDir))
            {
                if (usedFallbackDir)
                {
                    showTip(QStringLiteral("保存成功（已回退默认目录）"));
                }
                else
                {
                    showTip(QStringLiteral("已自动保存"));
                }
            }
            else
            {
                showTip(QStringLiteral("自动保存失败"));
            }
        }
        else if (saveCurrentImageWithDialog(decision.shouldRestoreWindow))
        {
            // 提示已在保存逻辑内处理。
        }

        dismissOverlay();
        endCaptureSession();
    });
}

void MainWindow::onOverlayPinRequested(const QRect &rect)
{
    QTimer::singleShot(90, this, [this, rect]()
    {
        const QPixmap pixmap = buildSelectionResultPixmap(rect);

        const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                    ? true
                                                    : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
        const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                                   ? CaptureResultDecision{shouldRestoreWindowByState, false}
                                                   : m_captureResultHandler->decide(CaptureResultAction::RegionConfirmed,
                                                                                    shouldRestoreWindowByState,
                                                                                    m_autoSaveEnabled);

        if (decision.shouldRestoreWindow)
        {
            restoreWindowIfNeeded();
        }

        if (pixmap.isNull())
        {
            dismissOverlay();
            showTip(QStringLiteral("贴图失败"));
            endCaptureSession();
            return;
        }

        applyRegionResult(pixmap, QStringLiteral("区域截图"), true);
        showPinnedImage(pixmap, QStringLiteral("区域截图贴图"));
        dismissOverlay();
        showTip(QStringLiteral("截图已贴到桌面顶层"));
        endCaptureSession();
    });
}
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void MainWindow::onOverlayLongCaptureToggled(bool enabled, const QRect &rect)
{
    if (m_longCaptureController == nullptr || m_overlay == nullptr)
    {
        return;
    }

    if (!enabled)
    {
        m_longCaptureController->cancel();
        return;
    }

    if (!m_longCaptureController->start(rect, m_overlay->winId()))
    {
        m_overlay->setLongCaptureModeEnabled(false);
        m_overlay->setStatusText(QStringLiteral("长截图启动失败"));
        showTip(QStringLiteral("长截图启动失败"));
        return;
    }

    m_overlay->setStatusText(QStringLiteral("长截图已开启"));
}

void MainWindow::onOverlayLongCaptureWheel(const QRect &rect, int delta)
{
    Q_UNUSED(rect)

    if (m_longCaptureController == nullptr)
    {
        return;
    }

    m_longCaptureController->requestManualScroll(delta);
}

void MainWindow::onOverlayLongCaptureSaveRequested(const QRect &rect)
{
    Q_UNUSED(rect)

    if (m_longCaptureController == nullptr)
    {
        return;
    }

    m_longCaptureController->saveAs();
}

void MainWindow::onOverlayLongCaptureConfirmRequested(const QRect &rect)
{
    Q_UNUSED(rect)

    if (m_longCaptureController == nullptr)
    {
        return;
    }

    m_longCaptureController->confirmCopy();
}

void MainWindow::onLongCapturePreviewUpdated(const QPixmap &pixmap)
{
    if (m_overlayClosing || pixmap.isNull())
    {
        return;
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setLongCapturePreview(pixmap);
    }

    updatePreview(pixmap);
}

void MainWindow::onLongCapturePredictedVisualHeightChanged(int height)
{
    if (m_overlayClosing)
    {
        return;
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setPredictedLongCaptureHeight(height);
    }
}

void MainWindow::onLongCaptureCommittedVisualHeightChanged(int height)
{
    if (m_overlayClosing)
    {
        return;
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setCommittedLongCaptureHeight(height);
    }
}

void MainWindow::onLongCaptureCaptureQualityChanged(CaptureQuality quality)
{
    if (m_overlayClosing)
    {
        return;
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setCaptureQuality(quality);
    }
}

void MainWindow::onLongCaptureStatusTextChanged(const QString &text)
{
    if (m_overlayClosing)
    {
        return;
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setStatusText(text);
    }
}

void MainWindow::onLongCaptureCopyReady(const QPixmap &pixmap)
{
    QPixmap finalPixmap = pixmap;
    if (!finalPixmap.isNull() && m_overlay != nullptr)
    {
        finalPixmap = m_overlay->applyEditsToPixmap(finalPixmap, QRect(QPoint(0, 0), finalPixmap.size()));
    }

    if (finalPixmap.isNull())
    {
        showTip(QStringLiteral("长截图复制失败"));
        resetLongCaptureState();
        endCaptureSession();
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    stopLongCaptureForClosingOverlay();
#endif
    dismissOverlay();

    const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                ? true
                                                : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
    const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                               ? CaptureResultDecision{shouldRestoreWindowByState, false}
                                               : m_captureResultHandler->decide(CaptureResultAction::LongCaptureCopied,
                                                                                shouldRestoreWindowByState,
                                                                                m_autoSaveEnabled);

    if (decision.shouldRestoreWindow)
    {
        restoreWindowIfNeeded();
    }

    updatePreview(finalPixmap);
    appendCaptureToHistory(finalPixmap, QStringLiteral("长截图"));
    copyPixmapToClipboard(finalPixmap);
    showTip(QStringLiteral("长截图已复制"));

    endCaptureSession();
}

void MainWindow::onLongCaptureSaveReady(const QPixmap &pixmap)
{
    QPixmap finalPixmap = pixmap;
    if (!finalPixmap.isNull() && m_overlay != nullptr)
    {
        finalPixmap = m_overlay->applyEditsToPixmap(finalPixmap, QRect(QPoint(0, 0), finalPixmap.size()));
    }

    if (finalPixmap.isNull())
    {
        showTip(QStringLiteral("长截图保存失败"));
        resetLongCaptureState();
        endCaptureSession();
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    stopLongCaptureForClosingOverlay();
#endif
    dismissOverlay();

    const bool shouldRestoreWindowByState = (m_uiStateCoordinator == nullptr)
                                                ? true
                                                : m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture();
    const CaptureResultDecision decision = (m_captureResultHandler == nullptr)
                                               ? CaptureResultDecision{shouldRestoreWindowByState, true}
                                               : m_captureResultHandler->decide(CaptureResultAction::LongCaptureSaveRequested,
                                                                                shouldRestoreWindowByState,
                                                                                m_autoSaveEnabled);

    if (decision.shouldRestoreWindow)
    {
        restoreWindowIfNeeded();
    }

    updatePreview(finalPixmap);
    appendCaptureToHistory(finalPixmap, QStringLiteral("长截图"));
    saveCurrentImageWithDialog(decision.shouldRestoreWindow);

    endCaptureSession();
}

void MainWindow::onLongCaptureFailed(const QString &message)
{
    if (m_overlay != nullptr)
    {
        m_overlay->setStatusText(message);
    }
}

#endif

void MainWindow::onModeFullClicked()
{
    m_captureMode = CaptureMode::Full;
    updateModeSegmentVisuals();
    showTip(QStringLiteral("模式：全屏"));
}

void MainWindow::onModeRegionClicked()
{
    m_captureMode = CaptureMode::Region;
    updateModeSegmentVisuals();
    showTip(QStringLiteral("模式：区域"));
}

void MainWindow::onPlaceholderAction()
{
    QAbstractButton *button = qobject_cast<QAbstractButton *>(sender());
    if (button == nullptr)
    {
        return;
    }

    if (button == ui->btnAutoSave)
    {
        m_autoSaveEnabled = ui->btnAutoSave->isChecked();
        saveAutoSaveEnabled();
        updateAutoSaveButtonText();
        showTip(m_autoSaveEnabled ? QStringLiteral("自动保存已开启")
                                  : QStringLiteral("自动保存已关闭"));
        return;
    }

    if (button == ui->btnFormatSetting || button == ui->btnMoreFormat)
    {
        toggleOutputFormat();
    }
}

void MainWindow::onOpenAutoSaveDirectoryRequested()
{
    const QString targetDirectory = QDir(effectiveAutoSaveDirectory()).absolutePath();
    QDir directory(targetDirectory);
    if (!directory.exists() && !directory.mkpath(QStringLiteral(".")))
    {
        showTip(QStringLiteral("自动保存目录创建失败"));
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(targetDirectory)))
    {
        showTip(QStringLiteral("无法打开自动保存目录"));
        return;
    }

    showTip(QStringLiteral("已打开自动保存目录"));
}

void MainWindow::onMoreRecentRequested()
{
    if (m_historyPage == HistoryPage::Gallery)
    {
        showHistoryPage(HistoryPage::Recent, false);
        return;
    }

    showHistoryPage(HistoryPage::Gallery, true);
}

void MainWindow::onHistoryPreviousRequested()
{
    if (m_currentHistoryIndex < 0)
    {
        return;
    }

    selectHistoryIndex(m_currentHistoryIndex + 1);
}

void MainWindow::onHistoryLatestRequested()
{
    selectLatestHistoryItem(true);
}

void MainWindow::onHistoryNextRequested()
{
    if (m_currentHistoryIndex <= 0)
    {
        return;
    }

    selectHistoryIndex(m_currentHistoryIndex - 1);
}
void MainWindow::onOpenSettingsRequested()
{
    const bool currentLaunchAtStartup = AutoStartManager::isEnabled();

    CaptureSettingsDialog dialog(this);
    dialog.setCurrentHotkey(m_captureHotkey);
    dialog.setAutoSaveDirectory(m_autoSaveDirectory);
    dialog.setLaunchAtStartupEnabled(currentLaunchAtStartup);
    dialog.setLaunchAtStartupSupported(AutoStartManager::isSupported());

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QKeySequence newHotkey = dialog.selectedHotkey();
    if (newHotkey != m_captureHotkey)
    {
        applyCaptureHotkey(newHotkey, true);
    }

    const QString newDirectory = dialog.selectedAutoSaveDirectory().trimmed();
    if (newDirectory != m_autoSaveDirectory)
    {
        m_autoSaveDirectory = newDirectory;
        saveAutoSaveDirectory();

        if (m_autoSaveDirectory.isEmpty())
        {
            showTip(QStringLiteral("自动保存目录已恢复默认"));
        }
        else
        {
            showTip(QStringLiteral("自动保存目录已更新"));
        }
    }

    const bool requestedLaunchAtStartup = dialog.selectedLaunchAtStartupEnabled();
    if (requestedLaunchAtStartup != currentLaunchAtStartup)
    {
        QString errorMessage;
        if (AutoStartManager::setEnabled(requestedLaunchAtStartup, &errorMessage))
        {
            SettingsService::writeBool(QStringLiteral("app/launch_at_startup"), requestedLaunchAtStartup);
            showTip(requestedLaunchAtStartup
                        ? QStringLiteral("已启用开机自启动")
                        : QStringLiteral("已关闭开机自启动"));
        }
        else
        {
            SettingsService::writeBool(QStringLiteral("app/launch_at_startup"), AutoStartManager::isEnabled());
            showTip(errorMessage.isEmpty()
                        ? QStringLiteral("开机自启动设置失败")
                        : errorMessage);
        }
        return;
    }

    SettingsService::writeBool(QStringLiteral("app/launch_at_startup"), currentLaunchAtStartup);
}

void MainWindow::onOpenAboutRequested()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onTrayShowRequested()
{
    if (m_uiStateCoordinator != nullptr)
    {
        m_uiStateCoordinator->markHiddenToTray(false);
    }

    showNormal();
    activateWindow();
    raise();
}

void MainWindow::onTrayExitRequested()
{
    if (m_uiStateCoordinator != nullptr)
    {
        m_uiStateCoordinator->markHiddenToTray(false);
    }

    m_forceQuit = true;
    if (m_trayIcon != nullptr)
    {
        m_trayIcon->hide();
    }
    qApp->quit();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
    {
        onTrayShowRequested();
    }
}

void MainWindow::setupRecentItems()
{
    if (m_captureHistoryManager == nullptr)
    {
        return;
    }

    if (!m_captureHistoryManager->initialize())
    {
        showTip(QStringLiteral("历史记录初始化失败"));
    }

    reloadRecentItems();
    showHistoryPage(HistoryPage::Recent, false);
}

void MainWindow::reloadRecentItems()
{
    refreshHistoryEntries(true);
    renderHistoryPages();
}

void MainWindow::refreshHistoryEntries(bool preserveSelection)
{
    const int previousIndex = m_currentHistoryIndex;
    QString selectedFilePath;
    if (preserveSelection && previousIndex >= 0 && previousIndex < m_historyEntries.size())
    {
        selectedFilePath = m_historyEntries.at(previousIndex).filePath;
    }

    if (m_captureHistoryManager == nullptr)
    {
        m_historyEntries.clear();
        m_currentHistoryIndex = -1;
        return;
    }

    m_captureHistoryManager->initialize();
    m_historyEntries = m_captureHistoryManager->entries();

    if (m_historyEntries.isEmpty())
    {
        m_currentHistoryIndex = -1;
        return;
    }

    if (!preserveSelection)
    {
        return;
    }

    if (!selectedFilePath.isEmpty())
    {
        for (int i = 0; i < m_historyEntries.size(); ++i)
        {
            if (m_historyEntries.at(i).filePath == selectedFilePath)
            {
                m_currentHistoryIndex = i;
                return;
            }
        }
    }

    if (previousIndex >= 0)
    {
        m_currentHistoryIndex = qMin(previousIndex, m_historyEntries.size() - 1);
        return;
    }

    m_currentHistoryIndex = -1;
}

void MainWindow::renderHistoryPages()
{
    clearHistoryLayout(ui->recentGridLayout);
    clearHistoryLayout(ui->galleryGridLayout);

    renderHistoryGrid(ui->recentGridLayout, ui->recentContentWidget, kRecentHistoryPreviewCount, true);
    renderHistoryGrid(ui->galleryGridLayout, ui->galleryContentWidget, -1, false);
    updateHistoryPageUi();
    updateHistoryNavigationButtons();
}

void MainWindow::clearHistoryLayout(QGridLayout *layout)
{
    if (layout == nullptr)
    {
        return;
    }

    while (layout->count() > 0)
    {
        QLayoutItem *item = layout->takeAt(0);
        if (item == nullptr)
        {
            continue;
        }

        if (item->widget() != nullptr)
        {
            item->widget()->deleteLater();
        }

        delete item;
    }
}

void MainWindow::renderHistoryGrid(QGridLayout *layout, QWidget *parentWidget, int maxItems, bool enableDragPin)
{
    if (layout == nullptr || parentWidget == nullptr)
    {
        return;
    }

    if (m_historyEntries.isEmpty())
    {
        QLabel *emptyLabel = new QLabel(QStringLiteral("暂无截图历史"), parentWidget);
        emptyLabel->setObjectName(QStringLiteral("recentEmptyLabel"));
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setMinimumHeight(280);
        layout->addWidget(emptyLabel, 0, 0, 1, kHistoryGridColumnCount);
        return;
    }

    const int itemCount = (maxItems < 0)
                              ? m_historyEntries.size()
                              : qMin(maxItems, m_historyEntries.size());
    for (int visibleIndex = 0; visibleIndex < itemCount; ++visibleIndex)
    {
        const CaptureHistoryManager::Entry &entry = m_historyEntries.at(visibleIndex);
        const int row = visibleIndex / kHistoryGridColumnCount;
        const int col = visibleIndex % kHistoryGridColumnCount;
        addHistoryItem(layout, parentWidget, entry, visibleIndex, row, col, enableDragPin);
    }
}

void MainWindow::addHistoryItem(QGridLayout *layout,
                                QWidget *parentWidget,
                                const CaptureHistoryManager::Entry &entry,
                                int historyIndex,
                                int row,
                                int col,
                                bool enableDragPin)
{
    if (layout == nullptr || parentWidget == nullptr)
    {
        return;
    }

    const bool selected = (historyIndex == m_currentHistoryIndex);
    const QPixmap thumbnail(entry.filePath);
    const QString title = entry.title.isEmpty() ? QStringLiteral("截图") : entry.title;
    const QString timeText = entry.createdAt.toString(QStringLiteral("yyyy.MM.dd HH:mm"));

    QFrame *card = new QFrame(parentWidget);
    card->setObjectName(QStringLiteral("recentItemCard"));
    card->setProperty("selected", selected);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(8);

    QToolButton *thumbButton = enableDragPin
                                   ? static_cast<QToolButton *>(new DraggableHistoryThumbButton(card))
                                   : static_cast<QToolButton *>(new QToolButton(card));
    thumbButton->setObjectName(QStringLiteral("recentThumbButton"));
    thumbButton->setCursor(Qt::PointingHandCursor);
    thumbButton->setMinimumSize(kHistoryThumbnailSize);
    thumbButton->setMaximumSize(kHistoryThumbnailSize);
    thumbButton->setAutoRaise(true);
    thumbButton->setCheckable(true);
    thumbButton->setChecked(selected);
    thumbButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QPixmap dragPreviewPixmap;
    if (!thumbnail.isNull())
    {
        const QPixmap scaled = thumbnail.scaled(kHistoryThumbnailSize,
                                                Qt::KeepAspectRatioByExpanding,
                                                Qt::SmoothTransformation);
        thumbButton->setIcon(QIcon(scaled));
        thumbButton->setIconSize(kHistoryThumbnailSize);
        dragPreviewPixmap = scaled;
    }
    else
    {
        thumbButton->setText(QStringLiteral("无预览"));
    }

    connect(thumbButton, &QToolButton::clicked, this, [this, historyIndex]()
    {
        selectHistoryIndex(historyIndex);
    });

    if (enableDragPin)
    {
        auto *draggableButton = static_cast<DraggableHistoryThumbButton *>(thumbButton);
        draggableButton->setDragPreviewPixmap(dragPreviewPixmap);
        const QString historyTitle = title;
        const QString historyTimeText = timeText;
        const QString filePath = entry.filePath;
        connect(draggableButton, &DraggableHistoryThumbButton::dragPinRequested, this, [this, filePath, historyTitle, historyTimeText](const QPoint &globalPos)
        {
            const QPixmap pinnedPixmap(filePath);
            if (pinnedPixmap.isNull())
            {
                showTip(QStringLiteral("历史图片不存在"));
                reloadRecentItems();
                return;
            }

            const QString pinnedTitle = QStringLiteral("%1 · %2").arg(historyTitle, historyTimeText);
            showPinnedImage(pinnedPixmap, pinnedTitle, globalPos);
            showTip(QStringLiteral("已从最近截图创建贴图"));
        });
    }

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("recentItemTitle"));

    QLabel *timeLabel = new QLabel(timeText, card);
    timeLabel->setObjectName(QStringLiteral("recentItemTime"));

    cardLayout->addWidget(thumbButton);
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(timeLabel);

    layout->addWidget(card, row, col);
}

bool MainWindow::selectHistoryIndex(int index, bool showFeedback)
{
    if (index < 0 || index >= m_historyEntries.size())
    {
        return false;
    }

    const CaptureHistoryManager::Entry entry = m_historyEntries.at(index);
    const QString title = entry.title.isEmpty() ? QStringLiteral("截图") : entry.title;
    const QPixmap preview(entry.filePath);
    if (preview.isNull())
    {
        showTip(QStringLiteral("历史图片不存在"));
        reloadRecentItems();
        return false;
    }

    m_currentHistoryIndex = index;
    updatePreview(preview);
    renderHistoryPages();

    if (showFeedback)
    {
        showTip(QStringLiteral("已加载历史截图：%1").arg(title));
    }

    return true;
}

void MainWindow::selectLatestHistoryItem(bool showFeedback)
{
    if (m_historyEntries.isEmpty())
    {
        if (showFeedback)
        {
            showTip(QStringLiteral("暂无截图历史"));
        }
        return;
    }

    selectHistoryIndex(0, showFeedback);
}

void MainWindow::showHistoryPage(HistoryPage page, bool refreshData)
{
    m_historyPage = page;

    if (refreshData)
    {
        reloadRecentItems();
        return;
    }

    updateHistoryPageUi();
    updateHistoryNavigationButtons();
}

void MainWindow::updateHistoryNavigationButtons()
{
    const bool hasHistory = !m_historyEntries.isEmpty();
    const bool hasCurrent = m_currentHistoryIndex >= 0 && m_currentHistoryIndex < m_historyEntries.size();

    ui->btnPrev->setEnabled(hasCurrent && (m_currentHistoryIndex + 1 < m_historyEntries.size()));
    ui->btnNext->setEnabled(hasCurrent && (m_currentHistoryIndex > 0));
    ui->btnShotIcon->setEnabled(hasHistory && (m_currentHistoryIndex != 0));
}

void MainWindow::updateHistoryPageUi()
{
    if (ui == nullptr)
    {
        return;
    }

    ui->historyStackedWidget->setCurrentWidget((m_historyPage == HistoryPage::Recent)
                                                   ? ui->recentPage
                                                   : ui->galleryPage);
    ui->recentTitleLabel->setText((m_historyPage == HistoryPage::Recent)
                                      ? QStringLiteral("最近截图")
                                      : QStringLiteral("截图图库"));
    ui->btnMoreRecent->setText((m_historyPage == HistoryPage::Recent)
                                   ? QStringLiteral("更多 >")
                                   : QStringLiteral("返回最近"));
}

QPixmap MainWindow::buildSelectionResultPixmap(const QRect &rect) const
{
    QPixmap pixmap = captureRegion(rect);
    if (!pixmap.isNull() && m_overlay != nullptr)
    {
        pixmap = m_overlay->applyEditsToPixmap(pixmap, rect);
    }

    return pixmap;
}

void MainWindow::applyRegionResult(const QPixmap &pixmap, const QString &title, bool copyToClipboard)
{
    if (pixmap.isNull())
    {
        return;
    }

    updatePreview(pixmap);
    if (copyToClipboard)
    {
        copyPixmapToClipboard(pixmap);
    }
    appendCaptureToHistory(pixmap, title);
}

void MainWindow::showPinnedImage(const QPixmap &pixmap, const QString &title, const QPoint &spawnGlobalPos)
{
    if (pixmap.isNull())
    {
        return;
    }

    prunePinnedImageWindows();

    const QString windowTitle = title.trimmed().isEmpty()
                                    ? QStringLiteral("截图贴图")
                                    : title.trimmed();
    auto *window = new PinnedImageWindow(pixmap, windowTitle, spawnGlobalPos);
    m_pinnedWindows.append(window);
    connect(window, &QObject::destroyed, this, [this]()
    {
        prunePinnedImageWindows();
    });

    window->show();
    window->raise();
    window->activateWindow();
}

void MainWindow::prunePinnedImageWindows()
{
    for (int index = m_pinnedWindows.size() - 1; index >= 0; --index)
    {
        if (m_pinnedWindows.at(index).isNull())
        {
            m_pinnedWindows.removeAt(index);
        }
    }
}
void MainWindow::appendCaptureToHistory(const QPixmap &pixmap, const QString &title)
{
    if (m_captureHistoryManager == nullptr)
    {
        return;
    }

    if (!m_captureHistoryManager->addCapture(pixmap, title))
    {
        showTip(QStringLiteral("写入截图历史失败"));
        return;
    }

    refreshHistoryEntries(false);
    if (!m_historyEntries.isEmpty())
    {
        m_currentHistoryIndex = 0;
    }
    renderHistoryPages();
}
void MainWindow::resetLongCaptureState()
{
    m_overlayClosing = false;
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    if (m_longCaptureController != nullptr)
    {
        m_longCaptureController->cancel();
    }
#endif
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void MainWindow::stopLongCaptureForClosingOverlay()
{
    if (m_longCaptureController == nullptr || m_longCaptureController->state() == LongCaptureSession::State::Idle)
    {
        return;
    }

    m_overlayClosing = true;
    m_longCaptureController->detachOverlay();
    m_longCaptureController->cancel();
}
#endif
void MainWindow::dismissOverlay()
{
    if (m_overlay == nullptr)
    {
        return;
    }

    SelectionOverlay *overlay = m_overlay;
    m_overlayClosing = true;
    m_overlay = nullptr;

    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    overlay->hide();
    overlay->close();
}

bool MainWindow::copyPixmapToClipboard(const QPixmap &pixmap) const
{
    if (pixmap.isNull())
    {
        return false;
    }

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard == nullptr)
    {
        return false;
    }

    const QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32).copy();
    if (image.isNull())
    {
        return false;
    }

    clipboard->setImage(image);
    return true;
}
void MainWindow::ensureTrayIcon()
{
    if (m_trayIcon != nullptr)
    {
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);
    QIcon trayIcon = windowIcon();
    if (trayIcon.isNull())
    {
        trayIcon = QIcon(QStringLiteral(":/icons/camera.svg"));
    }
    m_trayIcon->setIcon(trayIcon);
    m_trayIcon->setToolTip(QStringLiteral("轻影截图"));

    m_trayMenu = new QMenu(this);
    m_actionTrayShow = m_trayMenu->addAction(QStringLiteral("显示主窗口"));
    m_actionTrayExit = m_trayMenu->addAction(QStringLiteral("退出"));

    connect(m_actionTrayShow, &QAction::triggered, this, &MainWindow::onTrayShowRequested);
    connect(m_actionTrayExit, &QAction::triggered, this, &MainWindow::onTrayExitRequested);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
}

void MainWindow::updateModeSegmentVisuals()
{
    const QList<QToolButton *> modeButtons = {ui->modeFullButton, ui->modeRegionButton};

    for (QToolButton *button : modeButtons)
    {
        if (button->graphicsEffect() != nullptr)
        {
            button->graphicsEffect()->deleteLater();
            button->setGraphicsEffect(nullptr);
        }
    }

    QToolButton *selected = (m_captureMode == CaptureMode::Full) ? ui->modeFullButton : ui->modeRegionButton;
    selected->setChecked(true);

    auto *shadow = new QGraphicsDropShadowEffect(selected);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(126, 136, 154, 135));
    selected->setGraphicsEffect(shadow);
}

QPixmap MainWindow::captureRegion(const QRect &rect) const
{
    if (rect.width() <= 1 || rect.height() <= 1)
    {
        return QPixmap();
    }

    const QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty())
    {
        return QPixmap();
    }

    ShowTip::setCaptureSuppressed(true);

    bool restoreOverlay = false;
    if (m_overlay != nullptr && m_overlay->isVisible())
    {
        restoreOverlay = true;
        m_overlay->hide();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 20);
        QThread::msleep(18);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 20);
    }

    QImage stitched(rect.size(), QImage::Format_ARGB32_Premultiplied);
    stitched.fill(Qt::transparent);

    QPainter painter(&stitched);
    for (QScreen *screen : screens)
    {
        if (screen == nullptr)
        {
            continue;
        }

        const QRect screenRect = screen->geometry();
        const QRect intersected = rect.intersected(screenRect);
        if (!intersected.isValid())
        {
            continue;
        }

        const QPixmap fullScreenPixmap = screen->grabWindow(0);
        if (fullScreenPixmap.isNull())
        {
            continue;
        }

        const QRect sourceRect(intersected.topLeft() - screenRect.topLeft(), intersected.size());
        const QPoint targetPos = intersected.topLeft() - rect.topLeft();
        painter.drawPixmap(targetPos, fullScreenPixmap, sourceRect);
    }
    painter.end();

    if (restoreOverlay && m_overlay != nullptr)
    {
        m_overlay->show();
        m_overlay->raise();
        m_overlay->activateWindow();
        m_overlay->setFocus();
    }

    ShowTip::setCaptureSuppressed(false);
    return QPixmap::fromImage(stitched);
}
void MainWindow::updatePreview(const QPixmap &pixmap)
{
    m_currentPixmap = pixmap;

    if (m_currentPixmap.isNull())
    {
        ui->formatPreviewLabel->setPixmap(QPixmap());
        ui->formatPreviewLabel->setText(QStringLiteral("格式预览区域"));
        return;
    }

    const bool isLongPreview = m_currentPixmap.height() > m_currentPixmap.width() * 13 / 10;
    const Qt::AspectRatioMode aspectMode = isLongPreview
                                               ? Qt::KeepAspectRatio
                                               : Qt::KeepAspectRatioByExpanding;

    const QPixmap scaledMain = m_currentPixmap.scaled(ui->formatPreviewLabel->size(),
                                                      aspectMode,
                                                      Qt::SmoothTransformation);
    ui->formatPreviewLabel->setAlignment(isLongPreview ? (Qt::AlignTop | Qt::AlignHCenter)
                                                       : Qt::AlignCenter);
    ui->formatPreviewLabel->setPixmap(scaledMain);
    ui->formatPreviewLabel->setScaledContents(false);
    ui->startCaptureButton->setText(QStringLiteral("重新截图"));
}
void MainWindow::restoreWindowIfNeeded()
{
    if (m_uiStateCoordinator != nullptr && !m_uiStateCoordinator->shouldRestoreMainWindowAfterCapture())
    {
        return;
    }

    showNormal();
    activateWindow();
    raise();
}

void MainWindow::endCaptureSession()
{
    if (m_uiStateCoordinator != nullptr)
    {
        m_uiStateCoordinator->endCaptureSession();
    }
}

void MainWindow::showTip(const QString &text)
{
    if (m_tipPresenter != nullptr)
    {
        m_tipPresenter->show(text, TipPlacementPolicy::MousePreferredBottomRight, 2400);
    }
}

void MainWindow::loadAutoSaveEnabled()
{
    m_autoSaveEnabled = SettingsService::readBool(QStringLiteral("capture/auto_save_enabled"), true);
}

void MainWindow::saveAutoSaveEnabled() const
{
    SettingsService::writeBool(QStringLiteral("capture/auto_save_enabled"), m_autoSaveEnabled);
}

void MainWindow::updateAutoSaveButtonText()
{
    if (ui == nullptr)
    {
        return;
    }

    ui->btnAutoSave->setChecked(m_autoSaveEnabled);
    ui->btnAutoSave->setText(m_autoSaveEnabled ? QStringLiteral("自动保存：开启")
                                               : QStringLiteral("自动保存：关闭"));
}

void MainWindow::loadAutoSaveDirectory()
{
    m_autoSaveDirectory = SettingsService::readString(QStringLiteral("capture/auto_save_dir"), QStringLiteral(""))
                             .trimmed();
}

void MainWindow::saveAutoSaveDirectory() const
{
    SettingsService::writeString(QStringLiteral("capture/auto_save_dir"), m_autoSaveDirectory.trimmed());
}

QString MainWindow::defaultAutoSaveDirectory() const
{
    QString picturesDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (picturesDir.isEmpty())
    {
        picturesDir = QDir::homePath();
    }

    return QDir(picturesDir).filePath(QStringLiteral("轻影截图"));
}

QString MainWindow::effectiveAutoSaveDirectory() const
{
    const QString customDir = m_autoSaveDirectory.trimmed();
    if (!customDir.isEmpty())
    {
        return customDir;
    }

    return defaultAutoSaveDirectory();
}

bool MainWindow::autoSaveCapture(const QPixmap &pixmap,
                                 const QString &sceneTag,
                                 QString *savedPath,
                                 bool *usedFallbackDir) const
{
    if (pixmap.isNull())
    {
        return false;
    }

    const QString extension = outputFormatExtension();
    const QString formatName = outputFormatName();
    const QString tag = sceneTag.isEmpty() ? QStringLiteral("capture") : sceneTag;
    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));
    const QString baseName = QStringLiteral("%1_%2").arg(tag, timestamp);

    const QString primaryDir = effectiveAutoSaveDirectory();
    const QString fallbackDir = defaultAutoSaveDirectory();

    auto saveToDirectory = [&](const QString &targetDir, QString *outPath) -> bool
    {
        QDir dir;
        if (!dir.mkpath(targetDir))
        {
            return false;
        }

        QString targetPath;
        int suffix = 0;
        do
        {
            const QString fileName = (suffix == 0)
                                         ? QStringLiteral("%1.%2").arg(baseName, extension)
                                         : QStringLiteral("%1_%2.%3").arg(baseName).arg(suffix).arg(extension);
            targetPath = QDir(targetDir).filePath(fileName);
            ++suffix;
        }
        while (QFileInfo::exists(targetPath));

        const int quality = (m_outputFormat == OutputFormat::JPG) ? 95 : -1;
        if (!pixmap.save(targetPath, formatName.toUtf8().constData(), quality))
        {
            return false;
        }

        if (outPath != nullptr)
        {
            *outPath = targetPath;
        }

        return true;
    };

    if (usedFallbackDir != nullptr)
    {
        *usedFallbackDir = false;
    }

    if (saveToDirectory(primaryDir, savedPath))
    {
        return true;
    }

    if (QDir::cleanPath(primaryDir) == QDir::cleanPath(fallbackDir))
    {
        return false;
    }

    if (usedFallbackDir != nullptr)
    {
        *usedFallbackDir = true;
    }

    return saveToDirectory(fallbackDir, savedPath);
}

void MainWindow::loadCaptureHotkey()
{
    const QString configured = SettingsService::readString(QStringLiteral("capture/hotkey_sequence"),
                                                           QStringLiteral("Ctrl+Shift+A"));

    QKeySequence sequence = QKeySequence::fromString(configured, QKeySequence::PortableText);
    if (sequence.isEmpty())
    {
        sequence = QKeySequence(QStringLiteral("Ctrl+Shift+A"));
    }

    m_captureHotkey = sequence;
}

void MainWindow::saveCaptureHotkey() const
{
    SettingsService::writeString(QStringLiteral("capture/hotkey_sequence"),
                                 m_captureHotkey.toString(QKeySequence::PortableText));
}

bool MainWindow::applyCaptureHotkey(const QKeySequence &sequence, bool showFeedback)
{
    if (sequence.isEmpty())
    {
        if (showFeedback)
        {
            showTip(QStringLiteral("快捷键不能为空"));
        }
        return false;
    }

    if (m_globalHotkeyManager == nullptr)
    {
        if (showFeedback)
        {
            showTip(QStringLiteral("快捷键管理器不可用"));
        }
        return false;
    }

    if (!m_globalHotkeyManager->applyHotkey(sequence))
    {
        if (showFeedback)
        {
            const QString reason = m_globalHotkeyManager->lastError().isEmpty()
                                       ? QStringLiteral("全局快捷键注册失败")
                                       : m_globalHotkeyManager->lastError();
            showTip(reason);
        }
        return false;
    }

#ifndef Q_OS_WIN
    if (m_appHotkeyShortcut != nullptr)
    {
        m_appHotkeyShortcut->setKey(sequence);
    }
#endif

    m_captureHotkey = sequence;
    saveCaptureHotkey();
    updateHotkeyButtonText();

    if (showFeedback)
    {
        showTip(QStringLiteral("快捷键已更新：%1")
                    .arg(m_captureHotkey.toString(QKeySequence::NativeText)));
    }

    return true;
}

void MainWindow::updateHotkeyButtonText()
{
    if (ui == nullptr || ui->btnHotkeySetting == nullptr)
    {
        return;
    }

    const QString keyText = m_captureHotkey.toString(QKeySequence::NativeText);
    ui->btnHotkeySetting->setText(QStringLiteral("热键 %1").arg(keyText));
}

void MainWindow::loadOutputFormat()
{
    const QString format = SettingsService::readString(QStringLiteral("capture/output_format"),
                                                       QStringLiteral("PNG"))
                               .trimmed()
                               .toUpper();

    if (format == QStringLiteral("JPG") || format == QStringLiteral("JPEG"))
    {
        m_outputFormat = OutputFormat::JPG;
    }
    else
    {
        m_outputFormat = OutputFormat::PNG;
    }
}

void MainWindow::saveOutputFormat() const
{
    SettingsService::writeString(QStringLiteral("capture/output_format"), outputFormatName());
}

void MainWindow::updateFormatButtonText()
{
    if (ui == nullptr)
    {
        return;
    }

    ui->btnFormatSetting->setText(QStringLiteral("格式 %1")
                                      .arg((m_outputFormat == OutputFormat::PNG)
                                               ? QStringLiteral("PNG")
                                               : QStringLiteral("JPG")));
}

void MainWindow::toggleOutputFormat()
{
    m_outputFormat = (m_outputFormat == OutputFormat::PNG)
                         ? OutputFormat::JPG
                         : OutputFormat::PNG;

    updateFormatButtonText();
    saveOutputFormat();

    showTip(QStringLiteral("输出格式已切换为 %1")
                .arg((m_outputFormat == OutputFormat::PNG)
                         ? QStringLiteral("PNG")
                         : QStringLiteral("JPG")));
}

QString MainWindow::outputFormatExtension() const
{
    return (m_outputFormat == OutputFormat::PNG)
               ? QStringLiteral("png")
               : QStringLiteral("jpg");
}

QString MainWindow::outputFormatName() const
{
    return (m_outputFormat == OutputFormat::PNG)
               ? QStringLiteral("PNG")
               : QStringLiteral("JPG");
}




























