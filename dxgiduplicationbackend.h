/***********************************************************************************
*
* @file         dxgiduplicationbackend.h
* @brief        DXGI Desktop Duplication 长截图后端。
*
* @author       shanzhi
* @date         2026/03/09
* @history
***********************************************************************************/

#ifndef DXGIDUPLICATIONBACKEND_H
#define DXGIDUPLICATIONBACKEND_H

#include "desktopgdicapturebackend.h"
#include "longcapturebackend.h"

#ifdef Q_OS_WIN
struct IDXGIAdapter1;
struct IDXGIOutputDuplication;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct HMONITOR__;
using HMONITOR = HMONITOR__ *;
#endif

class DxgiDuplicationBackend : public LongCaptureBackend
{
public:
    DxgiDuplicationBackend();
    ~DxgiDuplicationBackend() override;

    QString backendName() const override;
    bool isAvailable() const override;
    bool beginObservation(const QRect &globalRect) override;
    CaptureFrame tryGetLatestFrame() override;
    void endObservation() override;
    QImage capture(const QRect &globalRect) override;

private:
#ifdef Q_OS_WIN
    bool ensureDuplicatorForRect(const QRect &globalRect);
    void resetDuplicator();
    bool acquireFrame(CaptureFrame *frame, int timeoutMs);
    QImage copyFrameToImage(ID3D11Texture2D *texture, const QRect &captureRect);
#endif

private:
    DesktopGdiCaptureBackend m_gdiFallback;
#ifdef Q_OS_WIN
    HMONITOR m_activeMonitor = nullptr;
    QRect m_activeOutputRect;
    bool m_runtimeAvailable = true;
    IDXGIAdapter1 *m_adapter = nullptr;
    ID3D11Device *m_device = nullptr;
    ID3D11DeviceContext *m_context = nullptr;
    IDXGIOutputDuplication *m_duplication = nullptr;
    ID3D11Texture2D *m_stagingTexture = nullptr;
#endif
};

#endif // DXGIDUPLICATIONBACKEND_H
