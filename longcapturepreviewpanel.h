/***********************************************************************************
*
* @file         longcapturepreviewpanel.h
* @brief        长截图右侧实时预览面板。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef LONGCAPTUREPREVIEWPANEL_H
#define LONGCAPTUREPREVIEWPANEL_H

#include <QFrame>
#include <QPixmap>

class QLabel;
class QScrollArea;
class QResizeEvent;

class LongCapturePreviewPanel : public QFrame
{
public:
    explicit LongCapturePreviewPanel(QWidget *parent = nullptr);

    void setPreview(const QPixmap &pixmap);
    void clearPreview();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void refreshPreview();

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_sizeLabel = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QLabel *m_imageLabel = nullptr;
    QPixmap m_sourcePixmap;
};

#endif // LONGCAPTUREPREVIEWPANEL_H
