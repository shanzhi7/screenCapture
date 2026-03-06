#include "mainwindow.h"

#include "longcapturestitcher.h"
#include "selectionoverlay.h"
#include "showtip.h"
#include "ui_mainwindow.h"

#include <QAbstractButton>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QEventLoop>
#include <QFile>
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
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QShortcut>
#include <QStringList>
#include <QSize>
#include <QSizePolicy>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tip(new ShowTip(this))
    , m_longCaptureDelayTimer(new QTimer(this))
    , m_longCaptureStitcher(std::make_unique<LongCaptureStitcher>())
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

    applyIcon(ui->sideCaptureButton, QStringLiteral(":/icons/camera.svg"), QSize(24, 24), QStringLiteral("C"));
    applyIcon(ui->sideGalleryButton, QStringLiteral(":/icons/folder.svg"), QSize(24, 24), QStringLiteral("G"));
    applyIcon(ui->sideConfigButton, QStringLiteral(":/icons/settings.svg"), QSize(24, 24), QStringLiteral("S"));
    applyIcon(ui->sideBottomButton, QStringLiteral(":/icons/settings.svg"), QSize(24, 24), QStringLiteral("S"));
    applyIcon(ui->btnPrev, QStringLiteral(":/icons/chevron_left.svg"), QSize(20, 20), QStringLiteral("<"));
    applyIcon(ui->btnShotIcon, QStringLiteral(":/icons/crop.svg"), QSize(20, 20), QStringLiteral("X"));
    applyIcon(ui->btnNext, QStringLiteral(":/icons/chevron_right.svg"), QSize(20, 20), QStringLiteral(">"));
    applyIcon(ui->btnHistory, QStringLiteral(":/icons/history.svg"), QSize(20, 20), QStringLiteral("H"));
    applyIcon(ui->btnNewTask, QStringLiteral(":/icons/plus_white.svg"), QSize(16, 16), QStringLiteral("+"));
    applyIcon(ui->btnTopSettings, QStringLiteral(":/icons/settings.svg"), QSize(18, 18), QStringLiteral("S"));

    ui->modeFullButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->modeRegionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->modeLayout->setContentsMargins(4, 4, 4, 4);
    ui->modeLayout->setSpacing(6);
    ui->modeLayout->setStretch(0, 1);
    ui->modeLayout->setStretch(1, 1);
    ui->modeScrollButton->hide();

    connect(ui->startCaptureButton, &QPushButton::clicked, this, &MainWindow::startCapture);
    connect(ui->modeFullButton, &QToolButton::clicked, this, &MainWindow::onModeFullClicked);
    connect(ui->modeRegionButton, &QToolButton::clicked, this, &MainWindow::onModeRegionClicked);
    connect(ui->btnHotkeySetting, &QPushButton::clicked, this, &MainWindow::startCapture);

    auto *shotShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+A")), this);
    shotShortcut->setContext(Qt::ApplicationShortcut);
    connect(shotShortcut, &QShortcut::activated, this, &MainWindow::startCapture);

    m_longCaptureDelayTimer->setSingleShot(true);
    m_longCaptureDelayTimer->setInterval(220);
    connect(m_longCaptureDelayTimer, &QTimer::timeout, this, [this]()
    {
        appendLongCaptureFrame();
        m_longCaptureWheelBusy = false;
    });

    const QList<QAbstractButton *> placeholderButtons = {
        ui->sideCaptureButton,
        ui->sideGalleryButton,
        ui->sideConfigButton,
        ui->sideBottomButton,
        ui->btnPrev,
        ui->btnShotIcon,
        ui->btnNext,
        ui->btnNewTask,
        ui->btnHistory,
        ui->btnTopSettings,
        ui->btnMoreRecent,
        ui->btnMoreFormat,
        ui->btnFormatSetting,
        ui->btnAutoSave
    };

    for (QAbstractButton *button : placeholderButtons)
    {
        connect(button, &QAbstractButton::clicked, this, &MainWindow::onPlaceholderAction);
    }

    setupRecentItems();
    updateModeSegmentVisuals();
    showTip(QStringLiteral("UI loaded"));
}

MainWindow::~MainWindow()
{
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

void MainWindow::startCapture()
{
    resetLongCaptureState();

    if (m_captureMode == CaptureMode::Full)
    {
        const QList<QScreen *> screens = QGuiApplication::screens();
        if (screens.isEmpty())
        {
            showTip(QStringLiteral("No screen available"));
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
                const QString name = screen->name().isEmpty() ? QStringLiteral("Screen %1").arg(i + 1) : screen->name();
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
            const QString selected = QInputDialog::getItem(this,
                                                           QStringLiteral("Select Screen"),
                                                           QStringLiteral("Choose a display:"),
                                                           items,
                                                           defaultIndex,
                                                           false,
                                                           &ok);
            if (!ok || selected.isEmpty())
            {
                showTip(QStringLiteral("Capture canceled"));
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

        const QPixmap pixmap = targetScreen->grabWindow(0);

        showNormal();
        activateWindow();

        if (pixmap.isNull())
        {
            showTip(QStringLiteral("Full capture failed"));
            return;
        }

        updatePreview(pixmap);
        showTip(QStringLiteral("Full capture done"));
        return;
    }

    if (m_overlay != nullptr)
    {
        m_overlay->close();
        m_overlay->deleteLater();
        m_overlay = nullptr;
    }

    m_overlay = new SelectionOverlay();
    connect(m_overlay, &SelectionOverlay::selectionFinished, this, &MainWindow::onSelectionFinished);
    connect(m_overlay, &SelectionOverlay::selectionCanceled, this, &MainWindow::onSelectionCanceled);
    connect(m_overlay, &SelectionOverlay::saveRequested, this, &MainWindow::onOverlaySaveRequested);
    connect(m_overlay, &SelectionOverlay::longCaptureToggled, this, &MainWindow::onOverlayLongCaptureToggled);
    connect(m_overlay, &SelectionOverlay::longCaptureWheel, this, &MainWindow::onOverlayLongCaptureWheel);
    connect(m_overlay, &SelectionOverlay::longCaptureSaveRequested, this, &MainWindow::onOverlayLongCaptureSaveRequested);
    connect(m_overlay, &SelectionOverlay::longCaptureConfirmRequested, this, &MainWindow::onOverlayLongCaptureConfirmRequested);

    connect(m_overlay, &QObject::destroyed, this, [this]()
    {
        m_overlay = nullptr;
        resetLongCaptureState();
    });

    hide();
    m_overlay->show();
    m_overlay->activateWindow();
    m_overlay->setFocus();
}

void MainWindow::saveCurrentImage()
{
    if (m_currentPixmap.isNull())
    {
        showTip(QStringLiteral("No image to save"));
        return;
    }

    const QString path = QFileDialog::getSaveFileName(this,
                                                      QStringLiteral("Save Capture"),
                                                      QStringLiteral("screenshot.png"),
                                                      QStringLiteral("PNG (*.png)"));
    if (path.isEmpty())
    {
        return;
    }

    showNormal();
    activateWindow();

    if (m_currentPixmap.save(path, "PNG"))
    {
        showTip(QStringLiteral("Saved"));
    }
    else
    {
        showTip(QStringLiteral("Save failed"));
    }
}

void MainWindow::onSelectionFinished(const QRect &rect)
{
    QTimer::singleShot(90, this, [this, rect]()
    {
        showNormal();
        activateWindow();

        const QPixmap pixmap = captureRegion(rect);
        if (pixmap.isNull())
        {
            showTip(QStringLiteral("Region capture failed"));
            return;
        }

        updatePreview(pixmap);
        QApplication::clipboard()->setPixmap(pixmap);
        showTip(QStringLiteral("Captured and copied"));
    });
}

void MainWindow::onSelectionCanceled()
{
    showNormal();
    activateWindow();
    showTip(QStringLiteral("Canceled"));
}

void MainWindow::onOverlaySaveRequested(const QRect &rect)
{
    QTimer::singleShot(90, this, [this, rect]()
    {
        const QPixmap pixmap = captureRegion(rect);

        showNormal();
        activateWindow();

        if (pixmap.isNull())
        {
            showTip(QStringLiteral("Save capture failed"));
            return;
        }

        updatePreview(pixmap);
        saveCurrentImage();
    });
}

void MainWindow::onModeFullClicked()
{
    m_captureMode = CaptureMode::Full;
    updateModeSegmentVisuals();
    showTip(QStringLiteral("Mode: Full"));
}

void MainWindow::onModeRegionClicked()
{
    m_captureMode = CaptureMode::Region;
    updateModeSegmentVisuals();
    showTip(QStringLiteral("Mode: Region"));
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
        const bool enabled = ui->btnAutoSave->isChecked();
        ui->btnAutoSave->setText(enabled ? QStringLiteral("Auto Save ON") : QStringLiteral("Auto Save OFF"));
        showTip(QStringLiteral("Auto save toggle reserved"));
        return;
    }

    if (button == ui->btnHotkeySetting || button == ui->startCaptureButton)
    {
        return;
    }

    const QString label = button->text().isEmpty() ? button->objectName() : button->text();
    showTip(QStringLiteral("Reserved: %1").arg(label));
}

void MainWindow::onOverlayLongCaptureToggled(bool enabled, const QRect &rect)
{
    if (!enabled)
    {
        resetLongCaptureState();
        showTip(QStringLiteral("Long capture off"));
        return;
    }

    m_longCaptureActive = true;
    m_longCaptureRect = rect;

    const QPixmap first = captureRegion(rect);
    if (first.isNull() || !m_longCaptureStitcher->begin(first.toImage()))
    {
        showTip(QStringLiteral("Long capture start failed"));
        resetLongCaptureState();
        return;
    }

    m_longCaptureVisualHeight = m_longCaptureStitcher->visualHeight();
    if (m_overlay != nullptr)
    {
        m_overlay->setLongCaptureVisualHeight(m_longCaptureVisualHeight);
    }

    updatePreview(first);
    showTip(QStringLiteral("Long capture on"));
}

void MainWindow::onOverlayLongCaptureWheel(const QRect &rect, int delta)
{
    if (!m_longCaptureActive || m_longCaptureWheelBusy)
    {
        return;
    }

    m_longCaptureRect = rect;
    m_longCaptureWheelBusy = true;

    if (m_overlay != nullptr)
    {
        m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }

    injectWheelToBackground(delta);

    QTimer::singleShot(40, this, [this]()
    {
        if (m_overlay != nullptr)
        {
            m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }
    });

    m_longCaptureDelayTimer->start();
}

void MainWindow::onOverlayLongCaptureSaveRequested(const QRect &rect)
{
    m_longCaptureRect = rect;
    finishLongCapture(true, false);
}

void MainWindow::onOverlayLongCaptureConfirmRequested(const QRect &rect)
{
    m_longCaptureRect = rect;
    finishLongCapture(false, true);
}

void MainWindow::resetLongCaptureState()
{
    m_longCaptureActive = false;
    m_longCaptureWheelBusy = false;
    m_longCaptureVisualHeight = 0;
    m_longCaptureRect = QRect();

    if (m_longCaptureDelayTimer != nullptr)
    {
        m_longCaptureDelayTimer->stop();
    }

    if (m_longCaptureStitcher)
    {
        m_longCaptureStitcher->reset();
    }

    if (m_overlay != nullptr)
    {
        m_overlay->setLongCaptureVisualHeight(0);
    }
}

void MainWindow::appendLongCaptureFrame()
{
    if (!m_longCaptureActive || m_longCaptureRect.width() <= 1 || m_longCaptureRect.height() <= 1)
    {
        return;
    }

    const QPixmap frame = captureRegion(m_longCaptureRect);
    if (frame.isNull())
    {
        return;
    }

    const int appended = m_longCaptureStitcher->append(frame.toImage());
    if (appended <= 0)
    {
        return;
    }

    m_longCaptureVisualHeight = m_longCaptureStitcher->visualHeight();
    if (m_overlay != nullptr)
    {
        m_overlay->setLongCaptureVisualHeight(m_longCaptureVisualHeight);
    }

    // Show live merged result to reflect true long capture progress.
    const QPixmap merged = m_longCaptureStitcher->resultPixmap();
    if (!merged.isNull())
    {
        updatePreview(merged);
    }
}

void MainWindow::finishLongCapture(bool saveToFile, bool copyToClipboard)
{
    if (m_longCaptureActive)
    {
        appendLongCaptureFrame();
    }

    const QPixmap stitched = m_longCaptureStitcher->resultPixmap();
    if (stitched.isNull())
    {
        showTip(QStringLiteral("Long capture failed"));
        resetLongCaptureState();
        return;
    }

    showNormal();
    activateWindow();

    updatePreview(stitched);

    if (copyToClipboard)
    {
        QApplication::clipboard()->setPixmap(m_currentPixmap);
        showTip(QStringLiteral("Long capture copied"));
    }

    if (saveToFile)
    {
        saveCurrentImage();
    }

    if (!saveToFile && !copyToClipboard)
    {
        showTip(QStringLiteral("Long capture done"));
    }

    resetLongCaptureState();
}

void MainWindow::injectWheelToBackground(int delta)
{
#ifdef Q_OS_WIN
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = static_cast<DWORD>(delta);
    SendInput(1, &input, sizeof(INPUT));
#else
    Q_UNUSED(delta)
#endif
}

void MainWindow::setupRecentItems()
{
    addRecentItem(ui->recentGridLayout, 0, 0, QStringLiteral("Desktop"), QStringLiteral("2024.10.21 14:30"), "#5A8AE6", "#99B7F8");
    addRecentItem(ui->recentGridLayout, 0, 1, QStringLiteral("Web"), QStringLiteral("2024.10.21 14:30"), "#ADB7C6", "#E4E9F2");
    addRecentItem(ui->recentGridLayout, 0, 2, QStringLiteral("Files"), QStringLiteral("2024.10.21 14:30"), "#B3C7DC", "#E8EFF7");
    addRecentItem(ui->recentGridLayout, 0, 3, QStringLiteral("Explorer"), QStringLiteral("2024.10.21 14:30"), "#BAC8DE", "#EAF0F8");
    addRecentItem(ui->recentGridLayout, 1, 0, QStringLiteral("Code"), QStringLiteral("2024.10.21 14:30"), "#2A3446", "#4A607E");
    addRecentItem(ui->recentGridLayout, 1, 1, QStringLiteral("Settings"), QStringLiteral("2024.10.21 14:30"), "#A6B8D0", "#DFE8F4");
    addRecentItem(ui->recentGridLayout, 1, 2, QStringLiteral("Library"), QStringLiteral("2024.10.21 14:30"), "#B1C5DE", "#E8EFF8");
    addRecentItem(ui->recentGridLayout, 1, 3, QStringLiteral("Program"), QStringLiteral("2024.10.21 14:30"), "#B4C7DF", "#E8EFF8");
}

void MainWindow::addRecentItem(QGridLayout *layout,
                               int row,
                               int col,
                               const QString &title,
                               const QString &timeText,
                               const QString &colorA,
                               const QString &colorB)
{
    QFrame *card = new QFrame(ui->recentContentWidget);
    card->setObjectName(QStringLiteral("recentItemCard"));

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(8);

    QLabel *thumb = new QLabel(card);
    thumb->setObjectName(QStringLiteral("recentThumb"));
    thumb->setMinimumSize(220, 120);
    thumb->setMaximumHeight(120);

    thumb->setStyleSheet(QStringLiteral("QLabel{border-radius:14px;border:1px solid rgba(168,188,215,0.55);"
                                        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);}")
                             .arg(colorA, colorB));

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("recentItemTitle"));

    QLabel *timeLabel = new QLabel(timeText, card);
    timeLabel->setObjectName(QStringLiteral("recentItemTime"));

    cardLayout->addWidget(thumb);
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(timeLabel);

    layout->addWidget(card, row, col);
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

    bool restoreOverlay = false;
    if (m_overlay != nullptr && m_overlay->isVisible())
    {
        restoreOverlay = true;
        m_overlay->hide();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 5);
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

    return QPixmap::fromImage(stitched);
}

void MainWindow::updatePreview(const QPixmap &pixmap)
{
    m_currentPixmap = pixmap;

    const QPixmap scaledMain = m_currentPixmap.scaled(ui->formatPreviewLabel->size(),
                                                      Qt::KeepAspectRatioByExpanding,
                                                      Qt::SmoothTransformation);
    ui->formatPreviewLabel->setPixmap(scaledMain);
    ui->formatPreviewLabel->setScaledContents(true);
    ui->startCaptureButton->setText(QStringLiteral("Retake"));
}

void MainWindow::showTip(const QString &text)
{
    m_tip->showText(text, this);
}


