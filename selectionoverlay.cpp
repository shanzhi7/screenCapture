#include "selectionoverlay.h"

#include <QButtonGroup>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QToolButton>
#include <QWheelEvent>

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : QWidget(parent)
{
    // 覆盖全部显示器并集，支持多屏/跨屏选区。
    QRect virtualDesktop;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
    {
        if (screen == nullptr)
        {
            continue;
        }

        virtualDesktop = virtualDesktop.isNull() ? screen->geometry() : virtualDesktop.united(screen->geometry());
    }

    if (virtualDesktop.isNull())
    {
        virtualDesktop = QRect(0, 0, 1280, 720);
    }

    setGeometry(virtualDesktop);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);

    ensureToolbar();
}

QRect SelectionOverlay::selectedRect() const
{
    return m_selectedRect;
}

void SelectionOverlay::setLongCaptureVisualHeight(int height)
{
    // 长截图期间，外部持续传入累计高度用于视觉反馈。
    m_longCaptureVisualHeight = qMax(0, height);

    if (m_hasSelection)
    {
        updateToolbarPosition();
        update();
    }
}

void SelectionOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }

    // 点击工具条时，交给工具条子控件处理，不重置选区。
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

    update();
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_selecting)
    {
        return;
    }

    m_endPos = event->pos();
    update();
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_selecting || event->button() != Qt::LeftButton)
    {
        return;
    }

    m_endPos = event->pos();
    m_selecting = false;

    const QRect rect = currentRect();
    if (rect.width() < 2 || rect.height() < 2)
    {
        resetSelection();
        return;
    }

    // 保存全局坐标选区并展示工具条。
    m_hasSelection = true;
    m_selectedRect = toGlobalRect(rect);
    m_longCaptureVisualHeight = rect.height();
    updateToolbarPosition();
    update();
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event)
{
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

    // 从半透明遮罩中挖出选区。
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
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("拖拽选择截图区域"));
        return;
    }

    // 选区阶段保留轻微蓝色描边，增强边界识别。
    painter.setPen(QPen(QColor(118, 176, 255, 170), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(selection.adjusted(0, 0, -1, -1), 4, 4);

    // 保留尺寸提示。
    const QString sizeText = QString::number(selection.width()) + QStringLiteral(" x ") + QString::number(selection.height());
    int textY = selection.top() - 28;
    if (textY < 8)
    {
        textY = selection.bottom() + 8;
    }

    const QRect textRect(selection.left(), textY, 150, 22);
    painter.fillRect(textRect, QColor(12, 19, 30, 220));
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, sizeText);
}

void SelectionOverlay::wheelEvent(QWheelEvent *event)
{
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
        // 长截图视觉效果：选区高度随滚动累计高度持续增高。
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

void SelectionOverlay::updateToolbarPosition()
{
    if (m_toolbar == nullptr || !m_hasSelection)
    {
        return;
    }

    const QRect selection = currentDisplayRect();
    const int toolbarWidth = m_toolbar->sizeHint().width();
    const int toolbarHeight = m_toolbar->sizeHint().height();

    int toolbarX = selection.center().x() - toolbarWidth / 2;
    toolbarX = qMax(10, qMin(width() - toolbarWidth - 10, toolbarX));

    int toolbarY = selection.top() - toolbarHeight - 12;
    if (toolbarY < 8)
    {
        toolbarY = selection.bottom() + 12;
    }
    if (toolbarY + toolbarHeight > height() - 8)
    {
        toolbarY = height() - toolbarHeight - 8;
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
    m_toolbar->setFixedHeight(46);

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
