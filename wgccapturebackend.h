/***********************************************************************************
*
* @file         wgccapturebackend.h
* @brief        Windows Graphics Capture 预留后端。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef WGCCAPTUREBACKEND_H
#define WGCCAPTUREBACKEND_H

#include "longcapturebackend.h"

class WgcCaptureBackend : public LongCaptureBackend
{
public:
    WgcCaptureBackend();
    ~WgcCaptureBackend() override;

    QString backendName() const override;
    bool isAvailable() const override;
    QImage capture(const QRect &globalRect) override;
};

#endif // WGCCAPTUREBACKEND_H
