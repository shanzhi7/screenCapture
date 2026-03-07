#include "selectionoverlay.h"

#include <QButtonGroup>
#include <QContextMenuEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QShortcut>
#include <QToolButton>
#include <QWheelEvent>

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : QWidget(parent)
{
    // 跨多屏覆盖：取所有屏幕几何并集作为截图叠加层范围。
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

    // 全局取消与确认快捷键。
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

void SelectionOverlay::setLongCaptureVisualHeight(int height)
{
    m_longCaptureVisualHeight = qMax(0, height);

    if (m_hasSelection)
    {
        updateToolbarPosition();
        update();
    }
}

void SelectionOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    // 右键始终取消截图，行为等价 ESC / ✕。
    if (event->button() == Qt::RightButton)
    {
        event->accept();
        emit selectionCanceled();
        close();
        return;
    }

    // 非左键事件吞掉，避免透传到底层应用。
    if (event->button() != Qt::LeftButton)
    {
        event->accept();
        return;
    }

    activateWindow();
    raise();
    setFocus();

    // 工具条区域交给子控件处理。
    if (m_toolbar != nullptr && m_toolbar->isVisible() && m_toolbar->geometry().contains(event->pos()))
    {
        QWidget::mousePressEvent(event);
        return;
    }

    m_selecting = true;
    m_hasSelection = false;
    m_longCaptureEnabled = false;
    m_longCaptureVisualHeight = 0;
    m_startPos = event->pos();
    m_endPos = m_startPos;

    if (m_btnLongCapture != nullptr)
    {
        m_btnLongCapture->setChecked(false);
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
    m_longCaptureVisualHeight = rect.height();
    updateToolbarPosition();

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

    const QRect selection = currentDisplayRect();
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
        painter.setPen(QColor(220, 230, 255, 160));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("拖动鼠标选择区域，右键或 ESC 取消"));
        return;
    }

    // 选区内部保持几乎透明，仅用于视觉强调，不写入最终截图成品。
    painter.fillRect(selection, QColor(0, 0, 0, 1));

    // 选区交互态保留轻微蓝色描边。
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

void SelectionOverlay::wheelEvent(QWheelEvent *event)
{
    if (event == nullptr)
    {
        return;
    }

    if (m_longCaptureEnabled && m_hasSelection)
    {
        const int delta = event->angleDelta().y();
        if (delta != 0)
        {
            emit longCaptureWheel(m_selectedRect, delta);
            event->accept();
            return;
        }
    }

    QWidget::wheelEvent(event);
}

void SelectionOverlay::contextMenuEvent(QContextMenuEvent *event)
{
    if (event != nullptr)
    {
        event->accept();
    }
}

QRect SelectionOverlay::currentRect() const
{
    return QRect(m_startPos, m_endPos).normalized();
}

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
        const int maxBottom = height() - 1;
        const int targetBottom = qMin(maxBottom, selection.top() + targetHeight - 1);
        selection.setBottom(targetBottom);
    }

    return selection;
}

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

    const QRect selection = currentDisplayRect();
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
}

void SelectionOverlay::ensureToolbar()
{
    if (m_toolbar != nullptr)
    {
        return;
    }

    m_toolbar = new QFrame(this);
    m_toolbar->setObjectName(QStringLiteral("overlayToolbar"));
    m_toolbar->setFixedHeight(48);

    QHBoxLayout *layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(6);

    auto createTool = [this, layout](const QString &text, bool checkable = false)
    {
        QToolButton *button = new QToolButton(m_toolbar);
        button->setText(text);
        button->setCheckable(checkable);
        button->setCursor(Qt::PointingHandCursor);
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

    m_btnLongCapture = createTool(QStringLiteral("长截图"), true);
    m_btnSave = createTool(QStringLiteral("保存"));
    m_btnCancel = createTool(QStringLiteral("✕"));
    m_btnConfirm = createTool(QStringLiteral("✓"));

    connect(m_btnLongCapture, &QToolButton::clicked, this, [this]()
    {
        if (!m_hasSelection)
        {
            m_btnLongCapture->setChecked(false);
            return;
        }

        m_longCaptureEnabled = m_btnLongCapture->isChecked();
        if (!m_longCaptureEnabled)
        {
            m_longCaptureVisualHeight = currentRect().height();
        }

        updateToolbarPosition();
        update();
        emit longCaptureToggled(m_longCaptureEnabled, m_selectedRect);
    });

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

        if (m_longCaptureEnabled)
        {
            emit longCaptureSaveRequested(m_selectedRect);
        }
        else
        {
            emit saveRequested(m_selectedRect);
        }

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
        "QToolButton:hover {"
        "background: rgba(126, 170, 255, 35);"
        "border-color: rgba(170, 205, 255, 90);"
        "}"
        "QToolButton:checked {"
        "background: rgba(114, 166, 255, 72);"
        "border-color: rgba(176, 214, 255, 120);"
        "}");

    m_toolbar->adjustSize();
    m_toolbar->hide();
}

void SelectionOverlay::resetSelection()
{
    m_hasSelection = false;
    m_longCaptureEnabled = false;
    m_longCaptureVisualHeight = 0;
    m_selectedRect = QRect();

    if (m_btnLongCapture != nullptr)
    {
        m_btnLongCapture->setChecked(false);
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

    if (m_longCaptureEnabled)
    {
        emit longCaptureConfirmRequested(m_selectedRect);
    }
    else
    {
        emit selectionFinished(m_selectedRect);
    }

    close();
}


