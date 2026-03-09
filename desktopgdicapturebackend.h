/***********************************************************************************
*
* @file         desktopgdicapturebackend.h
* @brief        Windows GDI 桌面抓帧后端实现。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef DESKTOPGDICAPTUREBACKEND_H
#define DESKTOPGDICAPTUREBACKEND_H

#include "longcapturebackend.h"

class DesktopGdiCaptureBackend : public LongCaptureBackend
{
public:
    DesktopGdiCaptureBackend();
    ~DesktopGdiCaptureBackend() override;

    QImage capture(const QRect &globalRect) override;
};

#endif // DESKTOPGDICAPTUREBACKEND_H
