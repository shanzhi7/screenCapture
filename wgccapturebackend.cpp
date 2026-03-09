#include "wgccapturebackend.h"

WgcCaptureBackend::WgcCaptureBackend()
{
}

WgcCaptureBackend::~WgcCaptureBackend()
{
}

QString WgcCaptureBackend::backendName() const
{
    return QStringLiteral("WGC");
}

bool WgcCaptureBackend::isAvailable() const
{
    return false;
}

QImage WgcCaptureBackend::capture(const QRect &globalRect)
{
    Q_UNUSED(globalRect)
    return QImage();
}
