#include "selectionoverlay.h"

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
#include "longcapturepreviewpanel.h"
#endif

#include <QContextMenuEvent>
#include <QEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QShortcut>
#include <QToolButton>
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
#include <QWheelEvent>
#endif

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : QWidget(parent)
{
    QRect virtualDesktop;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
    {
        if (screen == nullptr)
        {
            continue;
        }

        virtualDesktop = virtualDesktop.isNull()
                             ? screen->geometry()
                             : virtualDesktop.united(screen->geometry());
    }

    if (virtualDesktop.isNull())
    {
        virtualDesktop = QRect(0, 0, 1280, 720);
    }

    setGeometry(virtualDesktop);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);

    auto *cancelShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    cancelShortcut->setContext(Qt::ApplicationShortcut);
    connect(cancelShortcut, &QShortcut::activated, this, [this]()
    {
        emit selectionCanceled();
        close();
    });

    auto *confirmShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    confirmShortcut->setContext(Qt::ApplicationShortcut);
    connect(confirmShortcut, &QShortcut::activated, this, &SelectionOverlay::confirmSelection);

    auto *confirmShortcutEnter = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    confirmShortcutEnter->setContext(Qt::ApplicationShortcut);
    connect(confirmShortcutEnter, &QShortcut::activated, this, &SelectionOverlay::confirmSelection);

    ensureToolbar();
}

QRect SelectionOverlay::selectedRect() const
{
    return m_selectedRect;
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void SelectionOverlay::setLongCaptureModeEnabled(bool enabled)
{
    const bool targetEnabled = enabled && m_hasSelection;
    m_longCaptureEnabled = targetEnabled;

    if (m_btnLongCapture != nullptr)
    {
        const bool blocked = m_btnLongCapture->blockSignals(true);
        m_btnLongCapture->setChecked(targetEnabled);
        m_btnLongCapture->blockSignals(blocked);
    }

    if (!targetEnabled)
    {

        m_longCaptureAnchorBottomLocal = -1;
        if (m_previewPanel != nullptr)
        {
            m_previewPanel->hide();
        }
    }
    else
    {
        const QRect current = currentRect();
        m_longCaptureAnchorBottomLocal = current.bottom();
        m_longCaptureVisualHeight = qMax(current.height(), m_longCaptureVisualHeight);

        if (m_previewPanel != nullptr)
        {
            updatePreviewPanelPosition();
            m_previewPanel->show();
            m_previewPanel->raise();
        }
    }

    updateToolbarPosition();
    update();
}

void SelectionOverlay::setLongCaptureVisualHeight(int height)
{
    m_longCaptureVisualHeight = qMax(0, height);
    if (m_hasSelection)
    {
        updateToolbarPosition();
        updatePreviewPanelPosition();
        update();
    }
}

void SelectionOverlay::setLongCapturePreview(const QPixmap &preview)
{
    m_previewPixmap = preview;
    if (m_previewPanel == nullptr)
    {
        return;
    }

    if (m_previewPixmap.isNull())
    {
        m_previewPanel->clearPreview();
        if (!m_longCaptureEnabled)
        {
            m_previewPanel->hide();
        }
        return;
    }

    m_previewPanel->setPreview(m_previewPixmap);
    if (m_longCaptureEnabled)
    {
        updatePreviewPanelPosition();
        m_previewPanel->show();
        m_previewPanel->raise();
    }
}

void SelectionOverlay::setStatusText(const QString &text)
{
    if (m_statusLabel == nullptr)
    {
        return;
    }

    m_statusLabel->setText(text);
}

void SelectionOverlay::setCaptureDecorationsHidden(bool hidden)
{
    m_captureDecorationsHidden = hidden;
    update();
}
#endif

void SelectionOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        event->accept();
        emit selectionCanceled();
        close();
        return;
    }

    if (event->button() != Qt::LeftButton)
    {
        event->accept();
        return;
    }

    activateWindow();
    raise();
    setFocus();

    if (m_toolbar != nullptr && m_toolbar->isVisible() && m_toolbar->geometry().contains(event->pos()))
    {
        QWidget::mousePressEvent(event);
        return;
    }

    m_selecting = true;
    m_hasSelection = false;
    m_startPos = event->pos();
    m_endPos = m_startPos;
    m_selectedRect = QRect();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_longCaptureEnabled = false;
    m_longCaptureVisualHeight = 0;
    m_longCaptureAnchorBottomLocal = -1;
    m_previewPixmap = QPixmap();

    if (m_btnLongCapture != nullptr)
    {
        m_btnLongCapture->setChecked(false);
    }

    if (m_previewPanel != nullptr)
    {
        m_previewPanel->clearPreview();
        m_previewPanel->hide();
    }
#endif

    if (m_statusLabel != nullptr)
    {
        m_statusLabel->setText(QStringLiteral("拖动中"));
    }

    if (m_toolbar != nullptr)
    {
        m_toolbar->hide();
    }

    event->accept();
    update();
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (!m_selecting)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    m_endPos = event->pos();
    event->accept();
    update();
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        event->accept();
        return;
    }

    if (!m_selecting || event->button() != Qt::LeftButton)
    {
        event->accept();
        return;
    }

    m_endPos = event->pos();
    m_selecting = false;

    const QRect rect = currentRect();
    if (rect.width() < 2 || rect.height() < 2)
    {
        resetSelection();
        event->accept();
        return;
    }

    m_hasSelection = true;
    m_selectedRect = toGlobalRect(rect);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_longCaptureVisualHeight = rect.height();
    m_longCaptureAnchorBottomLocal = rect.bottom();
#endif

    if (m_statusLabel != nullptr)
    {
        m_statusLabel->setText(QStringLiteral("已选择区域"));
    }

    updateToolbarPosition();
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    updatePreviewPanelPosition();
#endif

    activateWindow();
    raise();
    setFocus();

    event->accept();
    update();
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        emit selectionCanceled();
        close();
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        confirmSelection();
        return;
    }

    QWidget::keyPressEvent(event);
}

void SelectionOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const bool hideCaptureDecorations = m_captureDecorationsHidden;
    const QRect selection = currentDisplayRect();
#else
    const bool hideCaptureDecorations = false;
    const QRect selection = currentRect();
#endif
    const bool validSelection = (m_selecting || m_hasSelection) && selection.width() > 1 && selection.height() > 1;

    QPainterPath overlayPath;
    overlayPath.addRect(rect());

    if (validSelection)
    {
        QPainterPath holePath;
        holePath.addRect(selection);
        overlayPath = overlayPath.subtracted(holePath);
    }

    painter.fillPath(overlayPath, QColor(0, 0, 0, 122));

    if (!validSelection)
    {
        if (!hideCaptureDecorations)
        {
            painter.setPen(QColor(220, 230, 255, 160));
                        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("\u62d6\u52a8\u9f20\u6807\u9009\u62e9\u533a\u57df\uff0c\u53f3\u952e\u6216 ESC \u53d6\u6d88"));
        }
        return;
    }

    painter.fillRect(selection, QColor(0, 0, 0, 1));

    if (hideCaptureDecorations)
    {
        return;
    }

    painter.setPen(QPen(QColor(118, 176, 255, 170), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(selection.adjusted(0, 0, -1, -1), 4, 4);

    const QString sizeText = QString::number(selection.width())
                             + QStringLiteral(" x ")
                             + QString::number(selection.height());

    int textY = selection.top() - 28;

    const QRect screenRect = screenLocalRectForSelection(selection);
    const int minTop = screenRect.top() + 8;
    const int maxBottom = screenRect.bottom() - 8;

    if (textY < minTop)
    {
        textY = selection.bottom() + 8;
    }

    if (textY + 22 > maxBottom)
    {
        textY = qMax(minTop, maxBottom - 22);
    }

    int textX = selection.left();
    if (textX + 150 > screenRect.right() - 4)
    {
        textX = qMax(screenRect.left() + 4, screenRect.right() - 150 - 4);
    }

    const QRect textRect(textX, textY, 150, 22);
    painter.fillRect(textRect, QColor(12, 19, 30, 220));
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, sizeText);
}

void SelectionOverlay::contextMenuEvent(QContextMenuEvent *event)
{
    if (event != nullptr)
    {
        event->accept();
    }
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void SelectionOverlay::wheelEvent(QWheelEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (m_longCaptureEnabled && m_hasSelection)
    {
        int delta = event->angleDelta().y();
        if (delta == 0)
        {
            delta = event->pixelDelta().y() * 8;
        }

        if (delta != 0)
        {
            emit longCaptureWheel(m_selectedRect, delta);
            event->accept();
            return;
        }
    }

    QWidget::wheelEvent(event);
}

bool SelectionOverlay::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event != nullptr && event->type() == QEvent::Wheel && m_longCaptureEnabled && m_hasSelection)
    {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        int delta = wheelEvent->angleDelta().y();
        if (delta == 0)
        {
            delta = wheelEvent->pixelDelta().y() * 8;
        }

        if (delta != 0)
        {
            emit longCaptureWheel(m_selectedRect, delta);
            wheelEvent->accept();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}
#endif

QRect SelectionOverlay::currentRect() const
{
    return QRect(m_startPos, m_endPos).normalized();
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
QRect SelectionOverlay::currentDisplayRect() const
{
    QRect selection = currentRect();
    if (!m_hasSelection)
    {
        return selection;
    }

    if (m_longCaptureEnabled)
    {
        const int targetHeight = qMax(selection.height(), m_longCaptureVisualHeight);
        const QRect screenRect = screenLocalRectForSelection(selection);

        int anchorBottom = m_longCaptureAnchorBottomLocal;
        if (anchorBottom < 0)
        {
            anchorBottom = selection.bottom();
        }

        anchorBottom = qBound(screenRect.top(), anchorBottom, screenRect.bottom());
        const int targetTop = anchorBottom - targetHeight + 1;
        const int clampedTop = qMax(screenRect.top(), targetTop);

        selection.setTop(clampedTop);
        selection.setBottom(anchorBottom);
    }

    return selection;
}
#endif

QRect SelectionOverlay::toGlobalRect(const QRect &rect) const
{
    return rect.translated(geometry().topLeft());
}

QRect SelectionOverlay::screenLocalRectForSelection(const QRect &selection) const
{
    const QPoint fallbackGlobalCenter = toGlobalRect(selection).center();
    QPoint globalCenter = fallbackGlobalCenter;

    if (m_selectedRect.isValid())
    {
        globalCenter = m_selectedRect.center();
    }

    QScreen *targetScreen = QGuiApplication::screenAt(globalCenter);
    if (targetScreen == nullptr)
    {
        targetScreen = QGuiApplication::screenAt(fallbackGlobalCenter);
    }

    if (targetScreen == nullptr)
    {
        targetScreen = QGuiApplication::primaryScreen();
    }

    if (targetScreen == nullptr)
    {
        return rect();
    }

    QRect localRect = targetScreen->geometry().translated(-geometry().topLeft());
    localRect = localRect.intersected(rect());

    if (!localRect.isValid() || localRect.width() < 40 || localRect.height() < 40)
    {
        return rect();
    }

    return localRect;
}

void SelectionOverlay::updateToolbarPosition()
{
    if (m_toolbar == nullptr || !m_hasSelection)
    {
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    const QRect selection = currentDisplayRect();
#else
    const QRect selection = currentRect();
#endif
    const QRect screenRect = screenLocalRectForSelection(selection);
    const int toolbarWidth = m_toolbar->sizeHint().width();
    const int toolbarHeight = m_toolbar->sizeHint().height();

    const int margin = 8;
    const int minX = screenRect.left() + margin;
    const int maxX = screenRect.right() - toolbarWidth - margin;

    int toolbarX = selection.center().x() - toolbarWidth / 2;
    toolbarX = qMax(minX, qMin(maxX, toolbarX));

    const int minY = screenRect.top() + margin;
    const int maxY = screenRect.bottom() - toolbarHeight - margin;

    int toolbarY = selection.top() - toolbarHeight - 12;
    if (toolbarY < minY)
    {
        toolbarY = selection.bottom() + 12;
    }

    if (toolbarY > maxY)
    {
        toolbarY = maxY;
    }

    if (toolbarY < minY)
    {
        toolbarY = minY;
    }

    m_toolbar->move(toolbarX, toolbarY);
    m_toolbar->show();
    m_toolbar->raise();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    updatePreviewPanelPosition();
#endif
}

void SelectionOverlay::ensureToolbar()
{
    if (m_toolbar != nullptr)
    {
        return;
    }

    m_toolbar = new QFrame(this);
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_toolbar->installEventFilter(this);
#endif
    m_toolbar->setObjectName(QStringLiteral("overlayToolbar"));
    m_toolbar->setFixedHeight(48);

    auto *layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(6);

    auto createTool = [this, layout](const QString &text, bool checkable = false)
    {
        auto *button = new QToolButton(m_toolbar);
        button->setText(text);
        button->setCheckable(checkable);
        button->setCursor(Qt::PointingHandCursor);
#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
        button->installEventFilter(this);
#endif
        layout->addWidget(button);
        return button;
    };

    m_toolGroup = new QButtonGroup(this);
    m_toolGroup->setExclusive(true);

    QToolButton *btnPen = createTool(QStringLiteral("画笔"), true);
    QToolButton *btnRect = createTool(QStringLiteral("矩形"), true);
    QToolButton *btnEllipse = createTool(QStringLiteral("圆形"), true);
    QToolButton *btnMosaic = createTool(QStringLiteral("马赛克"), true);
    QToolButton *btnText = createTool(QStringLiteral("文字"), true);

    m_toolGroup->addButton(btnPen);
    m_toolGroup->addButton(btnRect);
    m_toolGroup->addButton(btnEllipse);
    m_toolGroup->addButton(btnMosaic);
    m_toolGroup->addButton(btnText);

    layout->addSpacing(8);

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_btnLongCapture = createTool(QStringLiteral("长截图"), true);

    connect(m_btnLongCapture, &QToolButton::clicked, this, [this]()
    {
        if (!m_hasSelection)
        {
            m_btnLongCapture->setChecked(false);
            return;
        }

        m_longCaptureEnabled = m_btnLongCapture->isChecked();
        const QRect current = currentRect();

        if (m_longCaptureEnabled)
        {
            m_longCaptureAnchorBottomLocal = current.bottom();
            m_longCaptureVisualHeight = qMax(current.height(), m_longCaptureVisualHeight);
            if (m_previewPanel != nullptr)
            {
                if (m_previewPixmap.isNull())
                {
                    m_previewPanel->clearPreview();
                }
                updatePreviewPanelPosition();
                m_previewPanel->show();
                m_previewPanel->raise();
            }
            if (m_statusLabel != nullptr)
            {
                m_statusLabel->setText(QStringLiteral("长截图已开启"));
            }
        }
        else
        {
            m_longCaptureVisualHeight = current.height();
            m_longCaptureAnchorBottomLocal = -1;
            if (m_previewPanel != nullptr)
            {
                m_previewPanel->hide();
            }
            if (m_statusLabel != nullptr)
            {
                m_statusLabel->setText(QStringLiteral("长截图已关闭"));
            }
        }

        updateToolbarPosition();
        update();
        emit longCaptureToggled(m_longCaptureEnabled, m_selectedRect);
    });
#endif

    m_statusLabel = new QLabel(QStringLiteral("等待选区"), m_toolbar);
    m_statusLabel->setObjectName(QStringLiteral("overlayStatusLabel"));
    layout->addWidget(m_statusLabel);
    layout->addSpacing(8);

    m_btnSave = createTool(QStringLiteral("保存"));
    m_btnCancel = createTool(QStringLiteral("✕"));
    m_btnConfirm = createTool(QStringLiteral("✓"));

    connect(m_btnCancel, &QToolButton::clicked, this, [this]()
    {
        emit selectionCanceled();
        close();
    });

    connect(m_btnConfirm, &QToolButton::clicked, this, &SelectionOverlay::confirmSelection);

    connect(m_btnSave, &QToolButton::clicked, this, [this]()
    {
        if (!m_hasSelection)
        {
            return;
        }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
        if (m_longCaptureEnabled)
        {
            emit longCaptureSaveRequested(m_selectedRect);
            return;
        }
#endif

        emit saveRequested(m_selectedRect);
        close();
    });

    m_toolbar->setStyleSheet(
        "QFrame#overlayToolbar {"
        "background: rgba(14, 20, 30, 216);"
        "border: 1px solid rgba(132, 180, 255, 80);"
        "border-radius: 16px;"
        "}"
        "QToolButton {"
        "color: #EAF2FF;"
        "background: transparent;"
        "border: 1px solid transparent;"
        "border-radius: 10px;"
        "padding: 6px 10px;"
        "font: 10pt 'Microsoft YaHei UI';"
        "}"
        "QLabel#overlayStatusLabel {"
        "color: rgba(222, 236, 255, 210);"
        "padding: 0 8px;"
        "font: 9pt 'Microsoft YaHei UI';"
        "font-weight: 500;"
        "}"
        "QToolButton:hover {"
        "background: rgba(126, 170, 255, 35);"
        "border-color: rgba(170, 205, 255, 90);"
        "}"
        "QToolButton:checked {"
        "background: rgba(114, 166, 255, 72);"
        "border-color: rgba(176, 214, 255, 120);"
        "}");

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_previewPanel = new LongCapturePreviewPanel(this);
    m_previewPanel->installEventFilter(this);
    m_previewPanel->hide();
#endif

    m_toolbar->adjustSize();
    m_toolbar->hide();
}

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
void SelectionOverlay::updatePreviewPanelPosition()
{
    if (m_previewPanel == nullptr || !m_hasSelection || !m_longCaptureEnabled)
    {
        return;
    }

    const QRect selection = currentDisplayRect();
    const QRect screenRect = screenLocalRectForSelection(selection);

    const int panelWidth = m_previewPanel->width();
    const int panelHeight = m_previewPanel->height();
    const int margin = 10;

    int panelX = selection.right() + 12;
    if (panelX + panelWidth > screenRect.right() - margin)
    {
        panelX = selection.left() - panelWidth - 12;
    }

    if (panelX < screenRect.left() + margin)
    {
        panelX = screenRect.left() + margin;
    }

    int panelY = selection.top();
    const int minY = screenRect.top() + margin;
    const int maxY = qMax(minY, screenRect.bottom() - panelHeight - margin);
    panelY = qBound(minY, panelY, maxY);

    m_previewPanel->move(panelX, panelY);
}
#endif

void SelectionOverlay::resetSelection()
{
    m_hasSelection = false;
    m_selectedRect = QRect();

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    m_longCaptureEnabled = false;
    m_longCaptureVisualHeight = 0;
    m_longCaptureAnchorBottomLocal = -1;
    m_previewPixmap = QPixmap();

    if (m_btnLongCapture != nullptr)
    {
        m_btnLongCapture->setChecked(false);
    }

    if (m_previewPanel != nullptr)
    {
        m_previewPanel->clearPreview();
        m_previewPanel->hide();
    }
#endif

    if (m_statusLabel != nullptr)
    {
        m_statusLabel->setText(QStringLiteral("等待选区"));
    }

    if (m_toolbar != nullptr)
    {
        m_toolbar->hide();
    }

    update();
}

void SelectionOverlay::confirmSelection()
{
    if (!m_hasSelection)
    {
        return;
    }

#if SCREENCAPTURE_ENABLE_LONG_CAPTURE
    if (m_longCaptureEnabled)
    {
        emit longCaptureConfirmRequested(m_selectedRect);
        return;
    }
#endif

    emit selectionFinished(m_selectedRect);
    close();
}




