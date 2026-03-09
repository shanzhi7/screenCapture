#include "longcapturepreviewpanel.h"

#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>

LongCapturePreviewPanel::LongCapturePreviewPanel(QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("longCapturePreviewPanel"));
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFixedSize(300, 540);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(6);

    m_titleLabel = new QLabel(QStringLiteral("效果预览"), this);
    m_titleLabel->setObjectName(QStringLiteral("longCapturePreviewTitle"));

    m_sizeLabel = new QLabel(QStringLiteral("等待长截图"), this);
    m_sizeLabel->setObjectName(QStringLiteral("longCapturePreviewSize"));

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName(QStringLiteral("longCapturePreviewScrollArea"));
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setObjectName(QStringLiteral("longCapturePreviewImage"));
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setText(QStringLiteral("等待预览"));
    m_imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_scrollArea->setWidget(m_imageLabel);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_sizeLabel);
    layout->addWidget(m_scrollArea, 1);

    setStyleSheet(
        "QFrame#longCapturePreviewPanel {"
        "background: rgba(18, 24, 36, 232);"
        "border: 1px solid rgba(116, 170, 255, 96);"
        "border-radius: 14px;"
        "}"
        "QLabel#longCapturePreviewTitle {"
        "color: #DDEBFF;"
        "font: 10pt 'Microsoft YaHei UI';"
        "font-weight: 600;"
        "}"
        "QLabel#longCapturePreviewSize {"
        "color: rgba(210, 225, 248, 180);"
        "font: 9pt 'Microsoft YaHei UI';"
        "}"
        "QScrollArea#longCapturePreviewScrollArea {"
        "background: rgba(8, 12, 18, 210);"
        "border: 1px solid rgba(120, 172, 255, 72);"
        "border-radius: 10px;"
        "}"
        "QLabel#longCapturePreviewImage {"
        "background: transparent;"
        "color: #8EA9CC;"
        "padding: 6px;"
        "}");
}

void LongCapturePreviewPanel::setPreview(const QPixmap &pixmap)
{
    m_sourcePixmap = pixmap;
    refreshPreview();
}

void LongCapturePreviewPanel::clearPreview()
{
    m_sourcePixmap = QPixmap();
    m_committedHeight = 0;
    m_predictedHeight = 0;
    refreshPreview();
}

void LongCapturePreviewPanel::setVisualHeights(int committedHeight, int predictedHeight)
{
    m_committedHeight = qMax(0, committedHeight);
    m_predictedHeight = qMax(m_committedHeight, predictedHeight);
    refreshPreview();
}

void LongCapturePreviewPanel::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    refreshPreview();
}

void LongCapturePreviewPanel::refreshPreview()
{
    if (m_imageLabel == nullptr || m_scrollArea == nullptr || m_sizeLabel == nullptr)
    {
        return;
    }

    if (m_sourcePixmap.isNull())
    {
        m_sizeLabel->setText(QStringLiteral("等待长截图"));
        m_imageLabel->setPixmap(QPixmap());
        m_imageLabel->setText(QStringLiteral("等待预览"));
        return;
    }

    const QPixmap displayPixmap = buildDisplayPixmap();
    const QSize viewportSize = m_scrollArea->viewport()->size() - QSize(12, 12);
    const QSize targetSize(qMax(80, viewportSize.width()), qMax(120, viewportSize.height()));
    const QPixmap scaled = displayPixmap.scaled(targetSize,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation);

    QString sizeText = QStringLiteral("%1 x %2").arg(m_sourcePixmap.width()).arg(m_sourcePixmap.height());
    const int pendingHeight = qMax(0, m_predictedHeight - m_committedHeight);
    if (pendingHeight > 0)
    {
        sizeText += QStringLiteral("  预测 +%1 px").arg(pendingHeight);
    }

    m_sizeLabel->setText(sizeText);
    m_imageLabel->setText(QString());
    m_imageLabel->setPixmap(scaled);
}

QPixmap LongCapturePreviewPanel::buildDisplayPixmap() const
{
    if (m_sourcePixmap.isNull())
    {
        return QPixmap();
    }

    const int committedHeight = qMax(m_sourcePixmap.height(), m_committedHeight);
    const int predictedHeight = qMax(committedHeight, m_predictedHeight);
    const int pendingHeight = qMax(0, predictedHeight - committedHeight);
    if (pendingHeight <= 0)
    {
        return m_sourcePixmap;
    }

    QPixmap composed(m_sourcePixmap.width(), predictedHeight);
    composed.fill(Qt::transparent);

    QPainter painter(&composed);
    painter.fillRect(composed.rect(), QColor(8, 12, 18, 0));
    painter.drawPixmap(0, pendingHeight, m_sourcePixmap);

    painter.fillRect(QRect(0, 0, composed.width(), pendingHeight), QColor(78, 132, 204, 44));
    QPen pendingPen(QColor(156, 204, 255, 185), 1.0, Qt::DashLine);
    pendingPen.setDashPattern({5.0, 4.0});
    painter.setPen(pendingPen);
    painter.drawRect(QRect(0, 0, composed.width() - 1, pendingHeight - 1));

    painter.setPen(QColor(214, 232, 255, 190));
    painter.drawText(QRect(0, 0, composed.width(), pendingHeight),
                     Qt::AlignCenter,
                     QStringLiteral("预测区域"));
    painter.end();
    return composed;
}
