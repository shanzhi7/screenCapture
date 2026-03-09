#include "longcapturepreviewpanel.h"

#include <QLabel>
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

    const QSize viewportSize = m_scrollArea->viewport()->size() - QSize(12, 12);
    const QSize targetSize(qMax(80, viewportSize.width()), qMax(120, viewportSize.height()));
    const QPixmap scaled = m_sourcePixmap.scaled(targetSize,
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);

    m_sizeLabel->setText(QStringLiteral("%1 x %2").arg(m_sourcePixmap.width()).arg(m_sourcePixmap.height()));
    m_imageLabel->setText(QString());
    m_imageLabel->setPixmap(scaled);
}
