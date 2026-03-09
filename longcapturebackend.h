/***********************************************************************************
*
* @file         longcapturebackend.h
* @brief        长截图抓帧后端抽象。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef LONGCAPTUREBACKEND_H
#define LONGCAPTUREBACKEND_H

#include <QImage>
#include <QRect>

class LongCaptureBackend
{
public:
    virtual ~LongCaptureBackend() = default;

    // 抓取指定全局区域的桌面图像。
    virtual QImage capture(const QRect &globalRect) = 0;
};

#endif // LONGCAPTUREBACKEND_H
