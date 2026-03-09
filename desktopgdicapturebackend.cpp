#include "desktopgdicapturebackend.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

DesktopGdiCaptureBackend::DesktopGdiCaptureBackend()
{
}

DesktopGdiCaptureBackend::~DesktopGdiCaptureBackend()
{
}

QImage DesktopGdiCaptureBackend::capture(const QRect &globalRect)
{
#ifdef Q_OS_WIN
    if (globalRect.width() <= 1 || globalRect.height() <= 1)
    {
        return QImage();
    }

    HDC screenDc = GetDC(nullptr);
    if (screenDc == nullptr)
    {
        return QImage();
    }

    HDC memoryDc = CreateCompatibleDC(screenDc);
    if (memoryDc == nullptr)
    {
        ReleaseDC(nullptr, screenDc);
        return QImage();
    }

    BITMAPINFO bitmapInfo;
    ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = globalRect.width();
    bitmapInfo.bmiHeader.biHeight = -globalRect.height();
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void *bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(screenDc,
                                      &bitmapInfo,
                                      DIB_RGB_COLORS,
                                      &bits,
                                      nullptr,
                                      0);
    if (bitmap == nullptr || bits == nullptr)
    {
        DeleteDC(memoryDc);
        ReleaseDC(nullptr, screenDc);
        return QImage();
    }

    HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);
    BitBlt(memoryDc,
           0,
           0,
           globalRect.width(),
           globalRect.height(),
           screenDc,
           globalRect.x(),
           globalRect.y(),
           SRCCOPY);

    QImage image(static_cast<const uchar *>(bits),
                 globalRect.width(),
                 globalRect.height(),
                 QImage::Format_ARGB32);
    const QImage captured = image.copy();

    SelectObject(memoryDc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDc);
    ReleaseDC(nullptr, screenDc);
    return captured;
#else
    Q_UNUSED(globalRect)
    return QImage();
#endif
}


